#pragma once
#include <string>
#include <unordered_map>

std::unordered_map<std::string, std::string> loadSecrets(const std::string& filename);
void copy_cstr(char* dest, const std::string& src, size_t max_len);