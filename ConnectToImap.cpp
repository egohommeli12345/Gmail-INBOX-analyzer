#include <iostream>
#include <fstream>
#include <limits>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <algorithm>

//
// Created by Samuli on 10/29/24.
//
SSL_CTX *InitSSL()
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD *method = TLS_client_method();
    return SSL_CTX_new(method);
}

int ConnectSocket(const char *server, int port)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    // Convert port to string for getaddrinfo
    std::string port_str = std::to_string(port);

    // Resolve server address and port
    if (getaddrinfo(server, port_str.c_str(), &hints, &res) != 0) {
        std::cerr << "Error resolving hostname\n";
        exit(EXIT_FAILURE);
    }

    // Create socket
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        std::cerr << "Error creating socket\n";
        freeaddrinfo(res);  // Free resources
        exit(EXIT_FAILURE);
    }

    // Connect using the first result in the addrinfo list
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        std::cerr << "Connection failed\n";
        close(sockfd);
        freeaddrinfo(res);  // Free resources
        exit(EXIT_FAILURE);
    }

    // Clean up
    freeaddrinfo(res);
    return sockfd;
}

std::string Base64Encode(const std::string &input)
{
    // BIO = Basic Input/Output
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input.c_str(), input.length());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    std::string output(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    return output;
}

bool OAuthAuthenticate(SSL *ssl, const std::string &userEmail, const
std::string &accessToken)
{
    std::string authString = "user=" + userEmail + "\x01" + "auth=Bearer " +
                             accessToken + "\x01\x01";
    std::string base64AuthString = Base64Encode(authString);

    std::string authCommand = "A1 AUTHENTICATE XOAUTH2 " + base64AuthString +
                              "\r\n";

    if (SSL_write(ssl, authCommand.c_str(), authCommand.length()) <= 0) {
        std::cerr << "Error sending XOAUTH2 command\n";
        return false;
    }
    char response[4096];
    int bytes = SSL_read(ssl, response, sizeof(response) - 1);
    if (bytes > 0) {
        response[bytes] = 0;
        std::cout << "\nServer response:\n" << response;
    } else {
        std::cerr << "Error reading response\n";
        return false;
    }

    // Wait for OK response
    while (strstr(response, "A1 OK") == nullptr) {
        bytes = SSL_read(ssl, response, sizeof(response) - 1);
        if (bytes > 0) {
            response[bytes] = 0;
            std::cout << "\nServer response:\n" << response;
//            return true;
        }
    }
    return true;
}

void SendImapCommand(std::string &commandStr, SSL *ssl, char *responseBuf,
                     size_t bufferSize, std::ofstream *outputFile)
{
    int bytes;
    if (SSL_write(ssl, commandStr.c_str(),
                  static_cast<int>(commandStr.length())) <= 0) {
        std::cerr << "Error sending IMAP command\n";
        return;
    }

    while (strstr(responseBuf, "A2 OK") == nullptr) {
        bytes = SSL_read(ssl, responseBuf,
                         static_cast<int>(bufferSize - 1));
        if (bytes > 0) {
            responseBuf[bytes] = 0;
            std::cout << "Server response:\n" << responseBuf;
            *outputFile << responseBuf;
        } else {
            std::cerr << "Error\n";
        }
    }
    memset(responseBuf, 0, bufferSize);
    outputFile->close();
}

void InitializeImapConnection(std::string &email, std::string &accessToken)
{
    const char *server = "imap.gmail.com";
    int port = 993;
//    std::string userEmail = "eschwarz598@gmail.com";
//    std::string userEmail = "emicasp2001@gmail.com";

    SSL_CTX *ctx = InitSSL();
    if (ctx == nullptr) {
        std::cerr << "Failed to Initialize SSL\n";
        exit(EXIT_FAILURE);
    }

    int sockfd = ConnectSocket(server, port);

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);

    if (SSL_connect(ssl) <= 0) {
        std::cerr << "SSL connection failed\n";
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }
    std::cout << "Connected to server with SSL\n";

    // Open output file
    std::ofstream outputFile("output.txt");

    if (!outputFile.is_open()) {
        std::cerr << "Failed to open output file\n";
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    // Authenticate
    bool authStatus = OAuthAuthenticate(ssl, email, accessToken);

    // On success, send following commands
    if (authStatus) {
        std::string command = "A2 EXAMINE INBOX\r\n";
        char buffer[4096];
        SendImapCommand(command, ssl, buffer, sizeof(buffer), &outputFile);

        command = "A2 FETCH 1:* BODY.PEEK[HEADER.FIELDS (FROM)]\r\n";
        SendImapCommand(command, ssl, buffer, sizeof(buffer), &outputFile);
    } else {
        std::cout << "Authentication failed, exiting...\n";
    }

    SSL_shutdown(ssl);  // Perform an orderly SSL shutdown
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
    std::cout << "Everything cleaned up and closed\n";
}