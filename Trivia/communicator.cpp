#include "communicator.h"

#include <exception>
#include <iostream>
#include <string>
#include <thread>

#include "request.h"
#include "request_result.h"

Communicator::Communicator(std::shared_ptr<RequestHandlerFactory> handlerFactory) : m_handleFactory(handlerFactory)
{
}

Communicator::~Communicator()
{
    try
    {
	// the only use of the destructor should be for freeing 
	// resources that was allocated in the constructor
	closesocket(_socket);
    }
    catch (...) {}
}

void Communicator::bindAndListen()
{
    struct sockaddr_in sa = { 0 };

    // this server use TCP. that why SOCK_STREAM & IPPROTO_TCP
    // if the server use UDP we will use: SOCK_DGRAM & IPPROTO_UDP
    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (_socket == INVALID_SOCKET)
	throw std::exception(__FUNCTION__ " - socket");

    sa.sin_port = htons(LISTEN_PORT); // port that server will listen for
    sa.sin_family = AF_INET;   // must be AF_INET
    sa.sin_addr.s_addr = INADDR_ANY;    // when there are few ip's for the machine. We will use always "INADDR_ANY"

    // again stepping out to the global namespace
    // Connects between the socket and the configuration (port and etc..)
    if (bind(_socket, (struct sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR)
	throw std::exception(__FUNCTION__ " - bind");

    // Start listening for incoming requests of clients
    if (listen(_socket, SOMAXCONN) == SOCKET_ERROR)
	throw std::exception(__FUNCTION__ " - listen");
    std::cout << "Listening on port " << LISTEN_PORT << std::endl;
}

void Communicator::handleRequests()
{
    while (true)
    {
	// the main thread is only accepting clients 
	// and add then to the list of handlers
	std::cout << "Waiting for client connection request" << std::endl;

	// this accepts the client and create a specific socket from server to this client
	SOCKET client_socket = ::accept(_socket, NULL, NULL);

	if (client_socket == INVALID_SOCKET)
	    std::cout << "Invalid socket. Ignoring.." << std::endl;

	std::cout << "Client accepted. Server and client can speak" << std::endl;

	// Insert the client to the clients map
	m_clients[client_socket] = m_handleFactory->createLoginRequestHandler(); // Default start is the login request handler
	
	// Start thread for the new client
	startThreadForNewClient(client_socket);
    }
}

void Communicator::startThreadForNewClient(SOCKET clientSocket)
{
    // Create the client's thread and detach it
    std::thread clientThread(&Communicator::clientHandler, this, clientSocket);
    clientThread.detach();
}

void Communicator::clientHandler(SOCKET clientSocket)
{
    uint32_t bytesRecv = 0;
    uint32_t err = 0;
    uint32_t bufferSize = 0;

    // While the socket isn't invalid
    while (clientSocket != INVALID_SOCKET && err != SOCKET_ERROR)
    {
	Request req;
	RequestResult res;
	std::shared_ptr<char> buffer;

	// Get the request id which is 1 byte
	bytesRecv = recv(clientSocket, (char *)&(req.id), 1, 0);
	if (bytesRecv <= 0)
	{
	    break;
	}

	// Get the message size
	bytesRecv = recv(clientSocket, (char *)&bufferSize, 4, 0);
	if (bytesRecv <= 0)
	{
	    break;
	}

	// Get the message
	buffer = std::shared_ptr<char>(new char[bufferSize + 1]);
	bytesRecv = recv(clientSocket, buffer.get(), bufferSize, 0);
	if (bytesRecv <= 0)
	{
	    break;
	}

	// Set EOF
	buffer.get()[bufferSize] = 0;

	// Copy buffer contents to the request vector buffer
	req.buffer.assign(buffer.get(), buffer.get() + bufferSize + 1);

	// Check if the request is relevant to the current request handler
	if (m_clients[clientSocket]->isRequestRelevant(req))
	{
	    // Handle the request
	    res = m_clients[clientSocket]->handleRequest(req);

	    // Send the response to the client
	    err = send(clientSocket, res.response.data(), (int)res.response.size(), 0);
	    if (err == SOCKET_ERROR)
	    {
		break;
	    }
	}
    }

    // Remove the client from the client list
    m_clients.erase(m_clients.find(clientSocket));
}
