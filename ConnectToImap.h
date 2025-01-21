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

#ifndef CONNECTTOIMAP_H
#define CONNECTTOIMAP_H

SSL_CTX *InitSSL();

int ConnectSocket(const char *server, int port);

std::string Base64Encode(const std::string &input);

bool OAuthAuthenticate(SSL *ssl, const std::string &userEmail,
                       const std::string &accessToken);

void SendImapCommand(std::string &commandStr, SSL *ssl, char *responseBuf,
                     size_t bufferSize, std::ofstream *outputFile);

void InitializeImapConnection(std::string &email, std::string &accessToken);

#endif //CONNECTTOIMAP_H
