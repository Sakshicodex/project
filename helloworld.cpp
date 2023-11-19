#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

const int DEFAULT_PORT = 8081;

std::string readFile(const std::string& filename) {
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (file) {
        std::ostringstream contents;
        contents << file.rdbuf();
        file.close();
        return contents.str();
    }
    return "";
}

std::string getContentType(const std::string& path) {
    if (path.find(".html") != std::string::npos) {
        return "text/html";
    } else if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos) {
        return "image/jpeg";
    } else if (path.find(".png") != std::string::npos) {
        return "image/png";
    } else if (path.find(".css") != std::string::npos) {
        return "text/css";
    } else {
        return "application/octet-stream";
    }
}


void handleRequest(SOCKET clientSocket) {
    char buffer[1024];
    recv(clientSocket, buffer, sizeof(buffer), 0);

    std::istringstream request(buffer);
    std::string requestType, path;
    request >> requestType >> path;

    if (requestType == "GET") {
    if (path == "/") {
        path = "/index.html";
    }

    std::string fileContents = readFile("." + path);
    if (!fileContents.empty()) {
        std::string contentType = getContentType(path);
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\nContent-Length: " + std::to_string(fileContents.size()) + "\r\n\r\n" + fileContents;
        send(clientSocket, response.c_str(), response.size(), 0);
    } else {
        std::string notFoundResponse = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(clientSocket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
    }
}
 else {
        std::string notImplementedResponse = "HTTP/1.1 501 Not Implemented\r\n\r\n";
        send(clientSocket, notImplementedResponse.c_str(), notImplementedResponse.size(), 0);
    }

    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize winsock\n";
        return -1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(DEFAULT_PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "Server listening on port " << DEFAULT_PORT << "...\n";

    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed with error: " << WSAGetLastError() << "\n";
            closesocket(serverSocket);
            WSACleanup();
            return -1;
        }

        handleRequest(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
