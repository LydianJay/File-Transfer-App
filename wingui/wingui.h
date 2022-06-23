

#ifndef RENDER_SYS_H
#define RENDER_SYS_H
#include <Windows.h>

#define WG_MOUSE_ON_CLICK		0b10000000
#define WG_MOUSE_ON_HELD		0b01000000
#define WG_MOUSE_ON_RELEASE		0b00100000

#define WG_MOUSE_LEFT			0b00010000
#define WG_MOUSE_RIGHT			0b00001000

#define WG_KEY_PRESS			0b00000100
#define WG_KEY_RELEASED			0b00000010


#define WG_WINDOW_NO_RESIZE WS_VISIBLE | WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU
#define WG_WINDOW_DEFAULT WS_VISIBLE | WS_OVERLAPPEDWINDOW
#include <vector>
#include <gl\GL.h>
#include <gl\GLU.h>
#if defined (_WIN32) && defined(__cplusplus)
	#if defined (_MSC_VER)
	#pragma comment(lib, "opengl32.lib")
	#else
	#warning "Please link opengl libriaries "opengl32.lib" 
	#endif // _MSC_VER
#else
#error "For C++ Windows only!!!"
#endif

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext, const int *attribList);
typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

static wglChoosePixelFormatARB_type * wglChoosePixelFormatARB;
static wglCreateContextAttribsARB_type * wglCreateContextAttribsARB;
#define WG_GL_CONTEXT_CORE_PROFILE 0x00000001
#define WG_GL_CONTEXT_COMPAT_PROFILE 0x00000002


#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023
#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B


/*
-------LOG-----------
	Dec-23-2020
		-Added Mouse support
	Dec-27-2020
		-Remove some mouse functions
		-Remove some mouse related variables
		-Fix wrong mouse coordinates
		-Remove some debugging variables and functions
	January-5-2021
		-Added a blocking process message
		-Minor changes to function calls
		-Added GetButtonMessage function
		-Added Get text function and Get text length function
		-Added set text function
				*NOTE-TO-SELF: ButtonCall must be remove
	January-6-2021
		-Window has now have a resizeable option parameter
		-Added GetWindowSize function
	January-9-2021
		-Enable to specify background color upon creation of window
		-Added a color struct
		-Added bitmap
	January-27-2021
		-Window title function
	February-6-2021
		-Added File Explorer
	February-9-2021
		-Added Component classes  (Equivalent to JPanel in java)
			*Button classes for buttons
			*StaticText class for text
			*TextBox class for inputs
		-Added member functions for the new component class
	February-11-2021 
		-Added Creation of new sub windows
		*NEED to fix the resizing of sub windows
	February-16-2021
		-Fix the resizing problem on sub windows
		-All components (Buttons, textfield, ect.) are now handle in the Component class
			*The previous member function that does that are now remove
		-Added the BitMap component class
	March-3-2021
		-Added a modern way in creating an opengl context

--------END----------
*/

struct SSubWin
{
	int width = 0, heigth = 0;
	int x, y;
};

typedef unsigned char byte;

typedef struct vec3col
{
	byte r;
	byte g;
	byte b;

	vec3col();
	vec3col(float, float, float);
	vec3col(byte, byte, byte);
};
#define WG_WIN_COL_RED vec3col(1.0f, 0.0f, 0.0f)
#define WG_WIN_COL_BLUE vec3col(0.0f, 0.0f, 1.0f)
#define WG_WIN_COL_GREEN vec3col(0.0f, 1.0f, 0.0f)



class ICompInterface
{
protected:
	uint16_t width, heigth;
	uint16_t x, y;
	char * text = nullptr;
public:
	void Free();
	friend class CComponents;
	virtual int GetType() = 0;
	const char * GetText();
	SSubWin GetDim();

};

class CButton: public ICompInterface
{
	int message = INT_MIN;
public:
	friend class CComponents;
	const int GetButtonMessage();
	CButton(CButton& copy);
	CButton(uint16_t width, uint16_t heigth, uint16_t x, uint16_t y, const char * text, int message);
	~CButton();

public:
	
	int GetType();
};

class CStaticText : public ICompInterface
{
public:
	CStaticText(CStaticText& copy);
	CStaticText(uint16_t width, uint16_t heigth, uint16_t x, uint16_t y, const char * text);
	~CStaticText();

	int GetType();
};

class CTextbox : public ICompInterface
{
public:
	CTextbox(CTextbox& copy);
	CTextbox(uint16_t width, uint16_t heigth, uint16_t x, uint16_t y, const char * text);
	~CTextbox();
	int GetType();
};

class CBitMap : public ICompInterface
{
private:

public:
	CBitMap();
	CBitMap(CBitMap& copy);
	CBitMap(uint16_t width, uint16_t heigth, uint16_t x, uint16_t y, const char * text);
	int GetType();
};

class CComponents
{
private:
	HWND m_hHwnd = NULL;
	std::vector<HWND> m_hCompHandle;
	std::vector<ICompInterface *> m_vComponents;
	HWND * hParent = nullptr;
	friend class CWindow;
	friend int ICompInterface::GetType();
	friend const char * ICompInterface::GetText();
public:
	const char * GetText(unsigned int index);
	void GetTextField(unsigned int index,unsigned int count, char * buffer);
	void SetText(unsigned int index, const char * text);
	const int GetComponentCount();
	void Enable();
	void AddComponent(ICompInterface * component);
	void RemoveComponent(unsigned int index);
	void Disable();
	void DestroyComponents();
	~CComponents();
};




class CWindow
{
private:
	unsigned long m_wStyle = 0;
	static COORD m_WindowDim;
	HINSTANCE m_hInstance = nullptr;
	WNDCLASS m_wWndcls = {};
	bool m_bQuitstate = false;
	static CWindow * m_wWindow;



	std::vector<HWND> m_vSubWindows;
	int m_ButtonMessage = -1;
	std::vector<CComponents *> m_vCComponentsBound;
	std::vector<CComponents *> m_vSubWindowCompBound;
	std::vector<SSubWin> m_vSubWinSize;



	HWND m_hWinhwn = nullptr;
	HDC m_hdc;
	HGLRC m_hglrc;
	static bool m_bMouseDrag;
	MSG m_msg;
	unsigned int m_iWidth, m_iHeigth;
public:

protected:
	

private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
	static void(*KeyCall)(CWindow * window,unsigned char key);
	static void(*MouseCall)(CWindow *, float x, float y, unsigned char status, unsigned char mouse);
	static void(*ButtonOut)(CWindow *, int msg);
	static void(*ResizeCall)(int newWidth, int newHeigth);

public:
	
	CWindow(unsigned int width, unsigned int height, const char * windowname, unsigned long winmode, vec3col wincol);
	
	CWindow();
	CWindow(const CWindow&) = delete;
	~CWindow();

	void ProcessMessage();
	void ProcessMessageB();
	
	bool WindowShouldClose();
	void SwapWindowBuffers();
	
	void SetKeyCallFunc(void(*keycallback)(CWindow *, unsigned char));
	void SetResizeWindowCallFunc(void(*resizecall)(int, int));
	void SetMouseCallFunc(void(*mcall) (CWindow *, float x, float y, unsigned char, unsigned char));
	void SetWindowTitle(const char *title);

	void GetWindowSize(int *, int *);
	void ShowMyWindow();

	void CreateChildWindows(unsigned int width, unsigned int height, int x, int y, const char * windowname, unsigned long winmode, vec3col wincol);
	void OpenFileExplorer(char * buffer, size_t maxsize);
	const int GetButtonMessage();
	void InitComponents(unsigned int childwindowindex, CComponents * components);
	void InitComponents(CComponents * components);
	
	void HideWindow();

	


	/*----------for openGL-----------*/
	void CreateOpenGLContext();
	void CreateOpenGLContext(unsigned int glMajor, unsigned int glMinor, unsigned int glProfile);
	void DeleteOpenGLContext();
	/*-------------------------------*/


	
};






#endif