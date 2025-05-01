#include "secrets_loader.h"
#include <fstream>
#include <sstream>

std::unordered_map<std::string, std::string> loadSecrets(const std::string& filename) {
    std::unordered_map<std::string, std::string> secrets;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        secrets[key] = value;
    }

    return secrets;
}

void copy_cstr(char* dest, const std::string& src, size_t max_len) {
    size_t i = 0;
    for (; i < max_len - 1 && i < src.size(); ++i) {
        dest[i] = src[i];
    }
    dest[i] = '\0'; // null-terminate
}