#include "esp_tcp_server.h"
#include "WiFiServer.h"
#include "esp_uart.h"

EspTcpServer *EspTcpServer::_espTcpServer = nullptr;

EspTcpServer *EspTcpServer::get_instance()
{
    if (_espTcpServer == nullptr)
    {
        _espTcpServer = new EspTcpServer();
    }
    return _espTcpServer;
}

EspTcpServer::EspTcpServer()
{
    clientNumber = 0;
}

void EspTcpServer::startServer()
{
    server.begin();
}
void EspTcpServer::stopServer()
{
    server.end();
}
void EspTcpServer::getWiFiClient()
{
    if (server.hasClient())
    {
        // Check if a new client has connected
        serialUart.println("server.available()");
        WiFiClient newClient = server.available();
        if (newClient)
        {
            serialUart.println("new client");
            if (clientNumber < 5)
            {
                serialUart.println("New Client is Connected!");
                clients[clientNumber] = new WiFiClient(newClient);
                clientNumber++;
            }
            else
            {
                serialUart.println("Client Pool is Full");
            }
        }
        else
        {
            serialUart.println("New Client False");
        }
    }
}
void EspTcpServer::readTcpMessage()
{
    // Check whether each client has some data
    for (int i = 0; i < clientNumber; i++)
    {
        // If the client is in use, and has some data...
        while (clients[i]->available())
        {
            // Read the data
            char newChar = static_cast<char>(clients[i]->read());
            serialUart.print(newChar);
        }
    }
}
void EspTcpServer::sendTcpMessage(String message)
{   
    if (clientNumber > 0)
    {
        for (int i = 0; i < clientNumber; i++)
        {
            clients[i]->println(message);
        }
    }
}

// If there is a client which is not available anymore close it.
// And If needed shift the clients array
void EspTcpServer::checkClients(){
    for (int i = 0; i < clientNumber; i++)
    {
        if (!clients[i]->connected())
        {
            serialUart.print("Unconnected Client");
            serialUart.print(clientNumber);
            serialUart.println(i);
            //clients[i]->stop();
            clients[i] = NULL;
            if (clientNumber == (i + 1))
            {
                clientNumber--;
                serialUart.print("Client number: ");
                serialUart.println(i);
            }else
            {
                // Shifting the clients
                for (int j = i; j < clientNumber; j++)
                {
                    serialUart.print("Shift Array j: ");
                    serialUart.println(j);
                    serialUart.print("Client Number: ");
                    serialUart.println(clientNumber);
                    clients[j] = clients[j + 1];   
                }
                clientNumber--;
            }
        }
    }
}