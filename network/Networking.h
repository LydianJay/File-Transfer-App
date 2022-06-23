#ifndef _NETWORK_LIB_H
#define _NETWORK_LIB_H

#ifdef UNICODE
#undef UNICODE
#elif defined _UNICODE
#undef _UNICODE
#endif // UNICODE


#ifndef _WIN32
#warning Windows only
#endif // MSC_VER


#pragma comment(lib, "ws2_32.lib")
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include <string>

enum class NET_TYPE
{
	SERVER_TO_CLIENT,
	CLIENT_TO_SERVER
};

class INet {

private:
	WSADATA m_WSAData;
	sockaddr_in m_clientAdd;
	sockaddr_in m_serverAdd;
	std::thread m_comThread;
	bool m_isConnectionEstablised = false;
	SOCKET m_clientSocket;
	SOCKET m_winSock;
	unsigned short m_portNumber;
	sockaddr_in m_sockInfo;
	NET_TYPE m_netType;

private:
	void Receiving(void(*Receiver)(char * data, unsigned int nDataSize));
	void Listening();
	void Connecting();
public:


	/* 
	---------------------------------------------------------------------------------------------
		This method start a thread that waits for an 
		incomming connection to established
		should only be called once
	---------------------------------------------------------------------------------------------
	*/
	void ListenForConnection();
	
	/*
	---------------------------------------------------------------------------------------------
		This method returns true if a connection is establised
		ListforConnection() should be called first before querying

		-----------------------------------------------------------------------------------------
		@return - returns true if connection is established
		-----------------------------------------------------------------------------------------

	---------------------------------------------------------------------------------------------
	*/
	bool isEstablised();

	/*
	---------------------------------------------------------------------------------------------
		This method sends data the in the buffer to a established connection
		isEstablished() must be called first to check if connection exist

		--- this method may block the program ---

		-----------------------------------------------------------------------------------------
		@param data - a pointer to buffer that contains the data to be sent
		-----------------------------------------------------------------------------------------

		-----------------------------------------------------------------------------------------
		@param nDataSize - the size of the data in bytes
		-----------------------------------------------------------------------------------------

		-----------------------------------------------------------------------------------------
		@return - returns the number of bytes that was sent
		-----------------------------------------------------------------------------------------

	---------------------------------------------------------------------------------------------
	*/
	unsigned int sendData(char * data, unsigned int nDataSize);

	/*
	---------------------------------------------------------------------------------------------
		This method stops the thread from the ListenForConnection() method
		and uses the existing thread to try to catch any data that was received
		isEstablished() must be called first to check if connection exist
		
		-----------------------------------------------------------------------------------------
		@param void(*CallBackFunc)(char *, unsigned int) - this function pointer will be called if
		a data was received. the function will have a parameter that contains a pointer to buffer
		data and the size of the data in bytes
		-----------------------------------------------------------------------------------------

	---------------------------------------------------------------------------------------------

	*/
	void StartReceivingData(void (*CallBackFunc)(char * data, unsigned int nDataSize));

	/*
	---------------------------------------------------------------------------------------------
		This method will start a thread then tries to connect to a host
		it should ebe only called once. To check if connected call the isEstablished() method

		-----------------------------------------------------------------------------------------
		@param IPAdd - is a std::string that will contain the ip address of the host
		-----------------------------------------------------------------------------------------

	---------------------------------------------------------------------------------------------
	*/
	void ConnectToServer(std::string IPAdd);


	/*
	---------------------------------------------------------------------------------------------
		Closes the connection if there is a connection established
	---------------------------------------------------------------------------------------------
	*/
	void CloseConnection();

	INet();
	INet(NET_TYPE);
	
	~INet();
};



#endif // _NETWORK_LIB_H
