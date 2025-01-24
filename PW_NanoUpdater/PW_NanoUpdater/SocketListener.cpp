#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "34991"

int SocketListen(std::atomic<bool>& doWork)
{
   WSADATA wsaData;
   int iResult;

   SOCKET ListenSocket = INVALID_SOCKET;
   SOCKET ClientSocket = INVALID_SOCKET;

   struct addrinfo* result = NULL;
   struct addrinfo hints;

   int iSendResult;
   char recvbuf[DEFAULT_BUFLEN];
   int recvbuflen = DEFAULT_BUFLEN;
   // Initialize Winsock
   iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
   if (iResult != 0) {
      std::cerr << "WSAStartup failed with error: " << iResult << std::endl;
      return 1;
   }

   ZeroMemory(&hints, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_flags = AI_PASSIVE;

   // Resolve the server address and port
   iResult = getaddrinfo("localhost", DEFAULT_PORT, &hints, &result);
   if (iResult != 0) {
      std::cerr << "getaddrinfo failed with error: " << iResult << std::endl;
      WSACleanup();
      return 1;
   }

   // Create a SOCKET for the server to listen for client connections.
   ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
   if (ListenSocket == INVALID_SOCKET) {
      std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
      freeaddrinfo(result);
      WSACleanup();
      return 1;
   }

   // Setup the TCP listening socket
   iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
   if (iResult == SOCKET_ERROR) {
      std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl;
      freeaddrinfo(result);
      closesocket(ListenSocket);
      WSACleanup();
      return 1;
   }

   freeaddrinfo(result);

   iResult = listen(ListenSocket, SOMAXCONN);
   if (iResult == SOCKET_ERROR) {
      std::cerr << "listen failed with error: " << WSAGetLastError() << std::endl;
      closesocket(ListenSocket);
      WSACleanup();
      return 1;
   }

   // Accept a client socket
   ClientSocket = accept(ListenSocket, NULL, NULL);
   if (ClientSocket == INVALID_SOCKET) {
      std::cerr << "accept failed with error: " << WSAGetLastError() << std::endl;
      closesocket(ListenSocket);
      WSACleanup();
      return 1;
   }

   // No longer need server socket
   closesocket(ListenSocket);

   // Receive until the peer shuts down the connection
   do {

      iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
      if (iResult > 0) {
         std::string recievedData(recvbuf, iResult);
         std::cout << recievedData << std::endl;
      }
      else if (iResult == 0) {}
      else {
         std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
         closesocket(ClientSocket);
         WSACleanup();
         return 1;
      }

   } while (doWork.load());

   // shutdown the connection since we're done
   iResult = shutdown(ClientSocket, SD_SEND);
   if (iResult == SOCKET_ERROR) {
      std::cerr << "shutdown failed with error: " << WSAGetLastError() << std::endl;
      closesocket(ClientSocket);
      WSACleanup();
      return 1;
   }

   // cleanup
   closesocket(ClientSocket);
   WSACleanup();

   return 0;
}