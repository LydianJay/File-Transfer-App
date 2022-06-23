#include "wingui.h"


bool CWindow::m_bMouseDrag = false;
COORD CWindow::m_WindowDim = {0,0};
CWindow * CWindow::m_wWindow = nullptr;
void(*CWindow::ButtonOut)(CWindow *, int msg) = nullptr;
void(*CWindow::KeyCall)(CWindow * window, unsigned char key) = nullptr;
void(*CWindow::MouseCall)(CWindow *, float x, float y, unsigned char status, unsigned char mouse) = nullptr;
void(*CWindow::ResizeCall)(int newWidth, int newHeigth) = nullptr;

static const int VERSION_MINOR;


vec3col::vec3col()
{
	r = 255;
	g = 255;
	b = 255;
}

vec3col::vec3col(float R, float G, float B) :r(R * 255), g(G * 255), b(B * 255) {}

vec3col::vec3col(byte R, byte G, byte B) : r(R), g(G), b(B) {}


CWindow::CWindow(unsigned int width, unsigned int height, const char * windowname, unsigned long winmode, vec3col wincol)
{

	this->m_iHeigth = height;
	this->m_iWidth = width;
	m_WindowDim.X = (SHORT)width;
	m_WindowDim.Y = (SHORT)height;

	HBRUSH brush = CreateSolidBrush(RGB(wincol.r, wincol.g, wincol.b));
	m_wStyle = winmode;

	m_bQuitstate = false;

	m_hInstance = GetModuleHandle(NULL);

	m_wWndcls.hbrBackground = brush;
	m_wWndcls.hInstance = m_hInstance;
	m_wWndcls.lpfnWndProc = this->WndProc;
	m_wWndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	m_wWndcls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	m_wWndcls.lpszClassName = windowname;
	m_wWndcls.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	RegisterClass(&m_wWndcls);

	this->m_hWinhwn = CreateWindow(windowname, windowname, winmode, CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, m_hInstance, nullptr);
	CWindow::m_wWindow = this;
	ShowWindow(m_hWinhwn, SW_SHOW);

	

}
void CWindow::ProcessMessage()
{

	PeekMessage(&m_msg, this->m_hWinhwn, 0, 0, PM_REMOVE);
	TranslateMessage(&m_msg);
	DispatchMessage(&m_msg);

	/*if (m_msg.message == WM_QUIT)
		m_wWindow->m_bQuitstate = true;*/
}


//with blocking
void CWindow::ProcessMessageB()
{
	GetMessage(&m_msg, m_hWinhwn, 0, 0);
	TranslateMessage(&m_msg);
	DispatchMessage(&m_msg);
}

void CWindow::SetResizeWindowCallFunc(void(*resizecall)(int, int)) { ResizeCall = resizecall; }
void CWindow::SetMouseCallFunc(void(*mcall)(CWindow *, float x, float y, unsigned char, unsigned char)) { CWindow::MouseCall = mcall; }
CWindow::CWindow()
{
}


CWindow::~CWindow()
{
	this->DeleteOpenGLContext();
}



void CWindow::CreateOpenGLContext()
{
	m_hdc = GetDC(this->m_hWinhwn);
	
	PIXELFORMATDESCRIPTOR m_pxlform = { 0 };
	int m_iPxlFormat;
	m_pxlform = {
		sizeof(PIXELFORMATDESCRIPTOR),              // Size Of This Pixel Format Descriptor
		1,                                          // Version Number
		PFD_DRAW_TO_WINDOW |                        // Format Must Support CWindow
		PFD_SUPPORT_OPENGL |                        // Format Must Support OpenGL
		PFD_DOUBLEBUFFER,                           // Must Support Double Buffering
		PFD_TYPE_RGBA,                              // Request An RGBA Format
		32,                                         // Select Our Color Depth
		0, 0, 0, 0, 0, 0,                           // Color Bits Ignored
		8,                                          // No Alpha Buffer
		0,                                          // Shift Bit Ignored
		0,                                          // No Accumulation Buffer
		0, 0, 0, 0,                                 // Accumulation Bits Ignored
		16,                                         // 16Bit Z-Buffer (Depth Buffer)  
		0,                                          // No Stencil Buffer
		0,                                          // No Auxiliary Buffer
		PFD_MAIN_PLANE,                             // Main Drawing Layer
		0,                                          // Reserved
		0, 0, 0                                     // Layer Masks Ignored
	};


	m_iPxlFormat = ChoosePixelFormat(m_hdc, &m_pxlform);
	SetPixelFormat(m_hdc, m_iPxlFormat, &m_pxlform);
	m_hglrc = wglCreateContext(m_hdc);
	
	wglMakeCurrent(m_hdc, m_hglrc);
	
	
}

void CWindow::CreateOpenGLContext(unsigned int glMajor, unsigned int glMinor, unsigned int glProfile)
{
	CreateOpenGLContext();
	wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress("wglCreateContextAttribsARB");
	wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress("wglChoosePixelFormatARB");
	DeleteOpenGLContext();
	char windowname[128];
	windowname[127] = 0;
	int width, heigth;
	GetWindowSize(&width, &heigth);
	GetWindowText(m_hWinhwn, windowname, 127);
	DestroyWindow(m_hWinhwn);

	m_bQuitstate = false;

	m_hInstance = GetModuleHandle(NULL);

	m_wWndcls.hInstance = m_hInstance;
	m_wWndcls.lpfnWndProc = this->WndProc;
	m_wWndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	m_wWndcls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	m_wWndcls.lpszClassName = windowname;
	m_wWndcls.style = CS_OWNDC;
	RegisterClass(&m_wWndcls);

	this->m_hWinhwn = CreateWindow(windowname, windowname, m_wStyle, CW_USEDEFAULT, CW_USEDEFAULT, width, heigth, nullptr, nullptr, m_hInstance, nullptr);
	CWindow::m_wWindow = this;
	ShowWindow(m_hWinhwn, SW_SHOW);

	m_hdc = GetDC(this->m_hWinhwn);
	

	int pixel_format_attribs[] = {
		WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
		WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,         32,
		WGL_DEPTH_BITS_ARB,         24,
		WGL_STENCIL_BITS_ARB,       8,
		0
	};
	int pixelformat;
	UINT num_formats;
	wglChoosePixelFormatARB(m_hdc, pixel_format_attribs, 0, 1, &pixelformat, &num_formats);
	

	PIXELFORMATDESCRIPTOR pfd;
	DescribePixelFormat(m_hdc, pixelformat, sizeof(pfd), &pfd);
	SetPixelFormat(m_hdc, pixelformat, &pfd);
	

	int glattribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, glMajor,
		WGL_CONTEXT_MINOR_VERSION_ARB, glMinor,
		WGL_CONTEXT_PROFILE_MASK_ARB,  glProfile,
		0,
	};

	m_hglrc = wglCreateContextAttribsARB(m_hdc, NULL, glattribs);
	wglMakeCurrent(m_hdc, m_hglrc);
}

void CWindow::DeleteOpenGLContext()
{
	wglMakeCurrent(0, 0);
	wglDeleteContext(m_hglrc);
	ReleaseDC(CWindow::m_hWinhwn, m_hdc);
}




bool CWindow::WindowShouldClose()
{
	return m_bQuitstate;
}



void CWindow::SwapWindowBuffers()
{
	SwapBuffers(m_hdc);
}
void CWindow::SetKeyCallFunc(void(*keycallback)(CWindow *,unsigned char))
{
	KeyCall = keycallback;
}





void CWindow::ShowMyWindow()
{
	ShowWindow(this->m_hWinhwn, SW_SHOW);
}

void CWindow::HideWindow()
{
	ShowWindow(this->m_hWinhwn, SW_HIDE);
}
void CWindow::CreateChildWindows(unsigned int width, unsigned int height, int x, int y, const char * windowname, unsigned long winmode, vec3col wincol)
{
	SSubWin param;
	param.heigth = height;
	param.width = width;
	param.x = x;
	param.y = y;
	HBRUSH brush = CreateSolidBrush(RGB(wincol.r, wincol.g, wincol.b));
	m_bQuitstate = false;
	m_hInstance = GetModuleHandle(NULL);
	m_wWndcls.hbrBackground = brush;
	m_wWndcls.hInstance = m_hInstance;
	m_wWndcls.lpfnWndProc = this->WndProc;
	m_wWndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	m_wWndcls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	m_wWndcls.lpszClassName = windowname;
	m_wWndcls.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClass(&m_wWndcls);
	m_vSubWindows.push_back(CreateWindow(windowname, windowname, winmode | WS_CHILD, x, y, width, height, m_hWinhwn, nullptr, m_hInstance, nullptr));
	m_vSubWinSize.push_back(param);
	ShowWindow(m_hWinhwn, SW_SHOW);
}



const int CWindow::GetButtonMessage(){ return m_ButtonMessage;}



void CWindow::OpenFileExplorer(char * buffer, size_t maxsize)
{
	OPENFILENAME File = {0};
	File.hwndOwner = m_hWinhwn;
	File.lpstrFile = buffer;
	memset(File.lpstrFile, '\0', maxsize);
	File.lStructSize = sizeof(OPENFILENAME);
	File.nMaxFile = maxsize;

	GetOpenFileName(&File);
}


void CWindow::InitComponents(unsigned int childwindowindex, CComponents * components)
{
	int size = m_vSubWindows.size();
	if (size > 0 && childwindowindex < size)
	{
		components->hParent = &m_vSubWindows[childwindowindex];
		m_vSubWindowCompBound.push_back(components);
	}
}

void CWindow::InitComponents(CComponents * components)
{
	if (components != nullptr)
	{
		components->hParent = &m_hWinhwn;
		m_vCComponentsBound.push_back(components);
	}
}



void CWindow::GetWindowSize(int * width, int * heigth)
{
	if (width != nullptr && heigth != nullptr)
	{
		*width = m_iWidth;
		*heigth = m_iHeigth;
	}
}

void CWindow::SetWindowTitle(const char * title)
{
	SetWindowText(CWindow::m_hWinhwn, title);
}
LRESULT CALLBACK CWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	

	switch (msg)
	{

	case WM_CREATE:

		break;

	case WM_QUIT:
		
		break;

	case WM_SIZE:
	{
		if (m_wWindow != nullptr)
		{
			m_wWindow->m_iWidth = LOWORD(lp);
			m_wWindow->m_iHeigth = HIWORD(lp);

			int PrevWidth = m_wWindow->m_WindowDim.X;
			int PrevHeigth = m_wWindow->m_WindowDim.Y;

			float pX = (float)LOWORD(lp) / PrevWidth;
			float pY = (float)HIWORD(lp) / PrevHeigth;
			if (m_wWindow->ResizeCall != nullptr)ResizeCall(m_wWindow->m_iWidth, m_wWindow->m_iHeigth);

			if (hwnd == m_wWindow->m_hWinhwn)
			{
				for (size_t i = 0; i < m_wWindow->m_vSubWinSize.size(); i++)
				{

					SSubWin s = m_wWindow->m_vSubWinSize[i];
					if (PrevWidth != LOWORD(lp))
					{
						s.width *= pX;
						s.x *= pX;
					}
					if (PrevHeigth != HIWORD(lp))
					{
						s.heigth *= pY;
						s.y *= pY;
					}
					MoveWindow(m_wWindow->m_vSubWindows[i], s.x, s.y, s.width, s.heigth, TRUE);
				}
			}


			
		}

	}
	break;
	case WM_CLOSE:
		DestroyWindow(hwnd);

		if(m_wWindow->m_hWinhwn == hwnd)
		m_wWindow->m_bQuitstate = true;
		else
		{


			for (size_t i = 0; i < m_wWindow->m_vSubWindows.size(); i++)
			{
				if (m_wWindow->m_vSubWindows[i] == hwnd)
				{
					m_wWindow->m_vSubWindowCompBound[i]->DestroyComponents();
				}

			}

		}

		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	

	case WM_KEYDOWN:
		if (m_wWindow->KeyCall != nullptr)
			m_wWindow->KeyCall(m_wWindow, (unsigned char)wp);
		break;
	case WM_KEYUP:
		if(m_wWindow->KeyCall!=nullptr)
			m_wWindow->KeyCall(m_wWindow, (unsigned char)wp);
		break;

	case WM_COMMAND:
	{
		m_wWindow->m_ButtonMessage = wp;
	}
	break;


	
	
	
		
	case WM_LBUTTONDOWN:
	{
		
		POINT pt = { LOWORD(lp), HIWORD(lp) };
		if (DragDetect(hwnd, pt))
		{
			m_wWindow->m_bMouseDrag = true;
		}

		if (MouseCall != nullptr)
		{
			float fx = pt.x + (20 * (float)pt.x / m_wWindow->m_iWidth);
			float fy = pt.y + (40 * (float)pt.y / m_wWindow->m_iHeigth);


			if (m_wWindow->m_bMouseDrag)MouseCall(m_wWindow, fx , fy, WG_MOUSE_ON_HELD, WG_MOUSE_LEFT);
			else MouseCall(m_wWindow, fx, fy, WG_MOUSE_ON_CLICK, WG_MOUSE_LEFT);
		}

	}
	
	case WM_MOUSEMOVE:
		if (m_wWindow->m_bMouseDrag)
		{
			POINT pt = { LOWORD(lp), HIWORD(lp) };
			float fx = pt.x + (20 * (float)pt.x / m_wWindow->m_iWidth);
			float fy = pt.y + (40 * (float)pt.y / m_wWindow->m_iHeigth);

			if (MouseCall != nullptr)
			MouseCall(m_wWindow, fx, fy, WG_MOUSE_ON_HELD, WG_MOUSE_LEFT | WG_MOUSE_RIGHT);

		}
		break;
	case WM_LBUTTONUP:
	{
		if (m_wWindow->m_bMouseDrag)m_wWindow->m_bMouseDrag = false;

		if (MouseCall != nullptr) 
		{ 
			POINT pt = { LOWORD(lp), HIWORD(lp) };
			float fx = pt.x + (20 * (float)pt.x / m_wWindow->m_iWidth);
			float fy = pt.y + (40 * (float)pt.y / m_wWindow->m_iHeigth);
			MouseCall(m_wWindow, fx, fy, WG_MOUSE_ON_RELEASE, WG_MOUSE_LEFT);
		}
	}
		break;
	case WM_RBUTTONDOWN:
	{
		
		POINT pt = { LOWORD(lp), HIWORD(lp) };
		if (DragDetect(hwnd, pt))
		{
			m_wWindow->m_bMouseDrag = true;
		}

		if (MouseCall != nullptr)
		{
			float fx = pt.x + (20 * (float)pt.x / m_wWindow->m_iWidth);
			float fy = pt.y + (40 * (float)pt.y / m_wWindow->m_iHeigth);
			if (m_wWindow->m_bMouseDrag)MouseCall(m_wWindow, fx, fy, WG_MOUSE_ON_HELD, WG_MOUSE_RIGHT);
			else MouseCall(m_wWindow, fx, fy, WG_MOUSE_ON_CLICK, WG_MOUSE_RIGHT);
		}

	}
	case WM_RBUTTONUP:
	{
		if (m_wWindow->m_bMouseDrag)m_wWindow->m_bMouseDrag = false;

		if (MouseCall != nullptr)
		{
			POINT pt = { LOWORD(lp), HIWORD(lp) };
			float fx = pt.x + (20 * (float)pt.x / m_wWindow->m_iWidth);
			float fy = pt.y + (40 * (float)pt.y / m_wWindow->m_iHeigth);
			MouseCall(m_wWindow, fx, fy, WG_MOUSE_ON_RELEASE, WG_MOUSE_RIGHT);
		}
	}

	break;
	default:

		if (m_wWindow != nullptr)
			m_wWindow->m_ButtonMessage = -1;

		return DefWindowProc(hwnd, msg, wp, lp);
		break;
	}



	return DefWindowProc(hwnd, msg, wp, lp);
}





/*---------- PANELS ----------*/

const char * CComponents::GetText(unsigned int index)
{

	const unsigned int size = m_vComponents.size();
	if (size < 1 || index >= size)return nullptr;
	return m_vComponents[index]->GetText();
}

void CComponents::GetTextField(unsigned int index, unsigned int count, char * buffer)
{
	if (m_vComponents.size() < 1 || index >= m_vComponents.size() || buffer == nullptr)return;

	GetWindowText(m_hCompHandle[index],buffer, count);
}

void CComponents::SetText(unsigned int index, const char * text)
{
	if (m_vComponents.size() < 1 || index >= m_vComponents.size() || text == nullptr)return;
	SetWindowText(m_hCompHandle[index], text);
}

const int CComponents::GetComponentCount()
{
	return m_vComponents.size();
}

void CComponents::Enable()
{
	for (size_t i = 0; i < m_hCompHandle.size(); i++)
	{
		ShowWindow(m_hCompHandle[i], SW_SHOW);
	}
}


void CComponents::AddComponent(ICompInterface * component)
{
	
	if(hParent != nullptr)
	switch (component->GetType())
	{
	case 1:
	{
		CButton * button = dynamic_cast<CButton*>(component);

		m_vComponents.push_back(new CButton(*button));
		HWND hwnd = CreateWindow("Button", button->GetText(), WS_CHILD | WS_VISIBLE | WS_BORDER, component->x, component->y, component->width, component->heigth, *CComponents::hParent, (HMENU)button->message, nullptr, nullptr);
		if (hwnd != INVALID_HANDLE_VALUE)
			m_hCompHandle.push_back(hwnd);
		else
			m_hCompHandle.capacity();

		

		
	}
		break;

	case 2:
	{
		CStaticText * text = dynamic_cast<CStaticText *>(component);

		m_vComponents.push_back(new CStaticText(*text));

		m_hCompHandle.push_back(CreateWindow("Static", component->text, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, component->x, component->y, component->width, component->heigth, *CComponents::hParent, nullptr, nullptr, nullptr));
		
	}
		break;

	case 3:
	{
		CTextbox * text = dynamic_cast<CTextbox*>(component);

		m_vComponents.push_back(new CTextbox(*text));

		m_hCompHandle.push_back(CreateWindow("Edit", component->text, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, component->x, component->y, component->width, component->heigth, *CComponents::hParent, nullptr, nullptr, nullptr));
		
	}
	break;
	case 4:
	{
		CBitMap * bitmap = dynamic_cast<CBitMap *>(component);
		m_vComponents.push_back(new CBitMap(*bitmap));
		HBITMAP img = (HBITMAP)LoadImage(NULL, bitmap->text, IMAGE_BITMAP, component->width, component->heigth, LR_LOADFROMFILE);
		m_hCompHandle.push_back(CreateWindow("Static", NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP, component->x, component->y, component->width, component->heigth, *CComponents::hParent, nullptr, nullptr, nullptr));
		int size = m_hCompHandle.size();
		SendMessage(m_hCompHandle[size - 1], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)img);
		
	}
	break;

	default:
		break;
	}

	
}

void CComponents::RemoveComponent(unsigned int index)
{
	if (m_vComponents.size() > 0 && index < m_vComponents.size())
	{
		DestroyWindow(m_hCompHandle[index]);
		
		m_hCompHandle.erase(m_hCompHandle.begin() + index);
		m_vComponents[index]->Free();
		
		delete m_vComponents[index];
		m_vComponents.erase(m_vComponents.begin() + index);
		
	}
	
}



void CComponents::Disable()
{
	for (size_t i = 0; i < m_hCompHandle.size(); i++)
	{
		ShowWindow(m_hCompHandle[i], SW_HIDE);
	}

}

void CComponents::DestroyComponents()
{
	size_t size = m_vComponents.size();
	for (size_t i = 0; i < size; i++)
	{
		m_vComponents[i]->Free();
		delete m_vComponents[i];
		DestroyWindow(m_hCompHandle[i]);
	}

	m_vComponents.clear();
	m_hCompHandle.clear();
}

CComponents::~CComponents()
{
	for (size_t i = 0; i < m_vComponents.size(); i++)
	{
		m_vComponents[i]->Free();
		delete m_vComponents[i];
	}
}

const int CButton::GetButtonMessage()
{
	return message;
}

CButton::CButton(CButton & copy)
{
	if (copy.text != nullptr)
	{
		int size = strlen(copy.text) + 1;
		text = new char[size];
		memset(text, '\0', size);
		strcpy_s(text, size, copy.text);
	}


	width = copy.width;
	heigth = copy.heigth;
	x = copy.x;
	y = copy.y;
	message = copy.message;
}

CButton::CButton(uint16_t width, uint16_t heigth,uint16_t x, uint16_t y,const char * text, int message)
{
	int size = strlen(text);
	this->x = x;
	this->y = y;
	this->width = width;
	this->heigth = heigth;
	this->message = message;

	if (size > 1)
	{
		this->text = new char[size + 1];
		memset(this->text, '\0', size + 1);
		memcpy_s(this->text, size, text, size);
	}

}

CButton::~CButton()
{
	if (text != nullptr)delete text;
}

int CButton::GetType()
{
	return 1;
}

CStaticText::CStaticText(CStaticText & copy)
{
	if (copy.text != nullptr)
	{
		int size = strlen(copy.text) + 1;
		text = new char[size];
		memset(text, '\0', size);
		strcpy_s(text, size, copy.text);
	}


	width = copy.width;
	heigth = copy.heigth;
	x = copy.x;
	y = copy.y;
}

CStaticText::CStaticText(uint16_t width, uint16_t heigth, uint16_t x, uint16_t y, const char * text)
{
	int size = strlen(text);
	this->x = x;
	this->y = y;
	this->width = width;
	this->heigth = heigth;
	

	if (size > 1)
	{
		this->text = new char[size + 1];
		memset(this->text, '\0', size + 1);
		memcpy_s(this->text, size, text, size);
	}
}


CStaticText::~CStaticText()
{
	if (text != nullptr)delete text;
}

int CStaticText::GetType()
{
	return 2;
}

CTextbox::CTextbox(CTextbox & copy)
{
	if (copy.text != nullptr)
	{
		int size = strlen(copy.text) + 1;
		text = new char[size];
		memset(text, '\0', size);
		strcpy_s(text, size, copy.text);
	}


	width = copy.width;
	heigth = copy.heigth;
	x = copy.x;
	y = copy.y;
}

CTextbox::CTextbox(uint16_t width, uint16_t heigth, uint16_t x, uint16_t y, const char * text)
{
	int size = strlen(text);
	this->x = x;
	this->y = y;
	this->width = width;
	this->heigth = heigth;


	if (size > 1)
	{
		this->text = new char[size + 1];
		memset(this->text, '\0', size + 1);
		memcpy_s(this->text, size, text, size);
	}
}

CTextbox::~CTextbox()
{
	if (text != nullptr)delete[] text;
}

int CTextbox::GetType()
{
	return 3;
}

void ICompInterface::Free()
{
	if (text != nullptr)delete[] text;
}

const char * ICompInterface::GetText()
{
	return text;
}

SSubWin ICompInterface::GetDim()
{
	SSubWin s;
	s.x = x;
	s.y = y;
	s.heigth = heigth;
	s.width = width;
	return s;
}

CBitMap::CBitMap(){}

CBitMap::CBitMap(CBitMap & copy)
{
	if (copy.text != nullptr)
	{
		int size = strlen(copy.text) + 1;
		text = new char[size];
		memset(text, '\0', size);
		strcpy_s(text, size, copy.text);
	}


	width = copy.width;
	heigth = copy.heigth;
	x = copy.x;
	y = copy.y;
}

CBitMap::CBitMap(uint16_t width, uint16_t heigth, uint16_t x, uint16_t y, const char * text)
{
	int size = strlen(text);
	this->x = x;
	this->y = y;
	this->width = width;
	this->heigth = heigth;


	if (size > 1)
	{
		this->text = new char[size + 1];
		memset(this->text, '\0', size + 1);
		memcpy_s(this->text, size, text, size);
	}
}

int CBitMap::GetType()
{
	return 4;
}
