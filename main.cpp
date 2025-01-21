#include <iostream>
#include <fstream>
#include <limits>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>
#include "ConnectToImap.h"
#include "BasicServer.h"

bool cmp(std::pair<std::string, int> &pair1,
         std::pair<std::string, int> &pair2)
{
    return pair1.second > pair2.second;
}

void PrintSortedEmailAddressMap(std::unordered_map<std::string, int> &map)
{
    std::vector<std::pair<std::string, int>> A;

    for (auto &[key, value]: map) {
        A.emplace_back(key, value);
    }

    std::sort(A.begin(), A.end(), cmp);

    for (auto &item: A) {
        std::cout << item.second << " : " << item.first << "\n";
    }
}

int ReadFile(std::string &path, std::unordered_map<std::string, int> &map)
{
    int start = 0;
    char line[256];
    std::string emailAddress;
    std::ifstream is;

    is.open(path);

    while (!is.eof()) {
        is.getline(line, 256);

        if (is.fail() && !is.eof()) {
            is.clear();
            is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        if ((line[0] == 70 || line[0] == 102)
            && line[4] == 58 && line[5] == 32) {
            bool found = false;
            for (int i = 0; i < 256; i++) {
                if (line[i] == 64 && (line[i + 1] != 32 || line[i - 1] != 32)) {
                    found = true;
                    while (line[i] != 32 && line[i] != '<' && line[i] != 34
                           || i == 0) {
                        i--;
                    }
                    i++;
                    while (line[i] != 32 && line[i] != '>' && line[i] != 34 &&
                           line[i] != '\r' || i == 255) {
                        if (line[i] == 32 || line[i] == '>') continue;
                        emailAddress.push_back(line[i]);
                        i++;
                    }
                }

                if (line[i] == '\0' || found) {
                    if (!found) {
                        // Email not on the same row as "From: "
                        // so we read the next line and reset the iterator
//                        std::cout << line << "\n";
                        is.getline(line, 256);
                        i = 0;
                    } else {
                        break;
                    }
                }
            }
            start++;

            if (map.contains(emailAddress)) {
                map[emailAddress] += 1;
            } else {
                map.try_emplace(emailAddress, 1);
            }

            emailAddress.clear();
        }
        line[0] = '\0';
    }
    is.close();
    return start;
}

void InitializeEmailAnalyzing()
{
    std::unordered_map<std::string, int> emailMap;
    std::string path = "./output.txt";

    int emailCount = ReadFile(path, emailMap);

    PrintSortedEmailAddressMap(emailMap);

    std::cout << "\nTotal of " << emailCount << " emails analyzed."
              << std::endl;
    std::cout << "Unique addresses: " << emailMap.size() << std::endl;
}

typedef struct
{
    std::string emailAddress;
    std::string refresh_token;
    std::string access_token;
} Config;

Config ReadConfigFile()
{
    char buffer[512];
    Config cfg;
    std::string *fields[] = {&cfg.emailAddress,
                             &cfg.refresh_token,
                             &cfg.access_token};
    std::ifstream cfgFile("config.txt");
    if (cfgFile.is_open()) {
        int counter = 0;
        while (!cfgFile.eof() && counter <= 2) {
            cfgFile.getline(buffer, sizeof(buffer) - 1, '\n');
            *fields[counter] = buffer;
            counter++;
        }
    }
    std::cout << cfg.emailAddress << "\n";
    std::cout << cfg.refresh_token << "\n";
    std::cout << cfg.access_token << "\n";
    return cfg;
};

int main(int argc, char *argv[])
{

    /* TODO: Guide through account validation
     * TODO: Automatic access token feature
    */

    if (argc <= 1) {
        std::cout <<
                  "Commands:\n" <<
                  "-h help (this screen)\n" <<
                  "-u update (update email data)\n" <<
                  "-a analyze (analyze updated emails)\n";
        return 0;
    }

    Config cfg = ReadConfigFile();

    if (cfg.emailAddress.empty()) {
        std::cout << "Config file is empty\n"
                  << "Please use -c to configure settings.\n";
    }

    std::unordered_map<std::string, int> commandMap = {
            {"-a", 1},
            {"-u", 2},
            {"-t", 3},
            {"-h", 4},
    };

    int commandCode = commandMap[argv[1]];

    switch (commandCode) {
        case 1:
            InitializeEmailAnalyzing();
            break;
        case 2:
            InitializeImapConnection(cfg.emailAddress, cfg.access_token);
            break;
        case 3:
            std::cout << "Test case" << std::endl;
            ServerStart();
            break;
        case 4:
            std::cout <<
                      "Commands:\n" <<
                      "-h help (this screen)\n" <<
                      "-u update (update email data)\n" <<
                      "-a analyze (analyze updated emails)\n";
            break;
        default:
            std::cout << "Not implemented" << std::endl;
            break;
    }

    return 0;
}
