#include "network\Networking.h"
#include "wingui\wingui.h"
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>

class FileTransferProgram
{
private:
	CWindow * m_window = nullptr;
	CComponents m_comManager;
	std::vector<std::string> m_filePaths;
	std::string m_currentPath;
	INet * m_network = nullptr;
	bool m_connectionEstablised = false;
	std::thread m_waitThread;
	static FileTransferProgram* m_instance;
public:

	void main()
	{
		while (!m_window->WindowShouldClose())
		{
			ButtonHandler();
			

			m_window->ProcessMessageB();
			m_window->SwapWindowBuffers();
		}

	}

	void ButtonHandler()
	{

		switch (m_window->GetButtonMessage())
		{
		case 1:
		{
			char file[384];
			m_window->OpenFileExplorer(file, 384);
			m_currentPath = file;
			m_comManager.SetText(1, file);
		}
			
			break;
		case 2:
		{
			if (m_currentPath.size() > 0)
			{
				m_filePaths.push_back(m_currentPath);
				m_currentPath = "";
				std::stringstream ss;
				ss << m_filePaths.size() << ' ';
				m_comManager.SetText(5, ss.str().c_str());
			}

		}
			break;
		case 3:
		{
			
			char buffer[64];
			m_comManager.GetTextField(8, 64, buffer);
			std::string ip = buffer;
			SendFiles(ip);

		}
		
		default:
			break;
		}



	}

	FileTransferProgram()
	{
		m_window = new CWindow(480, 320, "File Transfer", WG_WINDOW_NO_RESIZE, { (byte)128, 128,128 });
		
		

		m_window->InitComponents(&m_comManager);
		m_comManager.AddComponent(&CStaticText(40, 20, 30, 40, "Files:"));
		m_comManager.AddComponent(&CStaticText(140, 20, 70, 40, ""));
		m_comManager.AddComponent(&CButton(120, 20, 220, 40, "Browse Files", 1));
		m_comManager.AddComponent(&CButton(120, 20, 220, 65, "Add", 2));
		m_comManager.AddComponent(&CStaticText(80, 22, 210, 95, "File Count: "));
		m_comManager.AddComponent(&CStaticText(40, 22, 292, 95, "0 "));
		m_comManager.AddComponent(&CButton(60, 30, 220, 165, "Send", 3));
		

		
		m_comManager.Enable();
		m_instance = this;
		
	
	}
	~FileTransferProgram()
	{
		if (m_window != nullptr)delete m_window;

		if (m_network != nullptr)delete m_network;


	}
private:
	

	


	void waitForServer()
	{
		if (m_instance == nullptr)return;
		else
		{
			if (m_instance->m_network == nullptr)return;
		}

		bool isEstablised = m_instance->m_network->isEstablised();
		while (!isEstablised)
		{
			isEstablised = m_instance->m_network->isEstablised();
		}


		m_instance->m_connectionEstablised = isEstablised;
	}

	void EstablishConnectionToServer(std::string ip)
	{
		if (!m_connectionEstablised)
		{
			if (m_network == nullptr)m_network = new INet(NET_TYPE::SERVER_TO_CLIENT);
			else delete m_network;


			m_network->ListenForConnection();
			if (m_waitThread.joinable())m_waitThread.join();

			m_waitThread = std::thread(&FileTransferProgram::waitForServer, this);

		}
		else
		{
			if (m_waitThread.joinable())m_waitThread.join();
		}
	}

	std::string GetFileName(std::string& filePath)
	{


		if (filePath.size() == 0)return std::string();

		int index = filePath.size() - 1;
		char comparator = filePath[index];
		while (comparator != '/' && comparator != '\\')
		{
			
			index--;

			comparator = filePath[index];
		}

		return std::string(&filePath.c_str()[index]);
	}

	void SendFiles(std::string ip)
	{

		if (m_filePaths.size() < 1)return;
		
		
		

		if (!m_connectionEstablised)EstablishConnectionToServer(ip);
		//MessageBox(NULL, "Connecting...", "Attempting to connect to server", MB_OK);
		while (!m_connectionEstablised)
		{
			std::cout << "Connecting...\n";
		}
		
		//MessageBox(NULL, "Connection Establised", "Sending files", MB_OK);
		unsigned int nFileSent = 0;
		for (std::string filePath : m_filePaths)
		{
			std::ifstream file(filePath, std::ios::binary);
			std::string fileName = GetFileName(filePath);


			if (!file.is_open()) 
			{ 
				std::cout << "error file open"; 
				continue; 
			}


			std::experimental::filesystem::path filesys(filePath);
			unsigned long long int fileSize = std::experimental::filesystem::file_size(filesys);

			int nBufferSize;

			if (fileSize > 10240) // 10mb
				nBufferSize = 10240;
			else
				nBufferSize = fileSize;
			
			char * buffer = new char[nBufferSize];
			unsigned long long int totalSize = fileSize;
			
			std::stringstream response;
			response << "HTTP/1.1 200 OK\r\n"
				<< "Connection: close\r\n"
				<< "Content-Type: application/octet-stream\r\n"
				<< "Content-Disposition: attachment; filename = \"\"\r\n"
				<< "Content-Length: " << totalSize << "\r\n"
				<< "\r\n";
			
			m_network->sendData(&response.str()[0], response.str().size());


			while (totalSize > 0)
			{
				int datSizeToSend = 0;
				if (totalSize >= nBufferSize)
				{
					file.read(buffer, nBufferSize);
					datSizeToSend = nBufferSize;

				}
				else
				{
					file.read(buffer, totalSize);
					datSizeToSend = totalSize;
				}

				m_network->sendData(buffer, datSizeToSend);
				totalSize -= nBufferSize;
			}
			nFileSent++;

			delete[] buffer;
		}
		
		std::stringstream ss;
		ss << "File sent: " << nFileSent;
		MessageBox(NULL, "Finish", ss.str().c_str(), MB_OK);
		m_network->CloseConnection();
		
	}

};
FileTransferProgram* FileTransferProgram::m_instance = nullptr;

int main()
{
	FileTransferProgram prog;
	prog.main();
	return 0;
}