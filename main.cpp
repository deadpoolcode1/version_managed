#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <xxhash.h>
#include <sys/utsname.h>
#include <cstring>

std::string compute_xxh32sum(const std::string &filepath)
{
    // Read the file
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return "";
    }

    std::vector<char> fileData((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());

    // Compute the hash
    uint32_t hash = XXH32(fileData.data(), fileData.size(), 0); // 0 is the seed

    // Convert hash to string for comparison
    std::stringstream ss;
    ss << std::hex << hash; // Use std::hex to get hexadecimal output

    return ss.str();
}

std::string getKernelModuleVersion(const std::string &modulePath)
{
    std::array<char, 128> buffer;
    std::string result;
    std::string cmd = "/sbin/modinfo -F vermagic " + modulePath + " 2>/dev/null";
    
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        std::cerr << "Failed to run modinfo command." << std::endl;
        return "";
    }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        result += buffer.data();
    }
    pclose(pipe);
    
    // Parsing the version from the vermagic output
    std::stringstream ss(result);
    std::getline(ss, result, ' '); // We only need the first part which is the version
    
    return result;
}

std::string getKernelModuleBasePath() {
    struct utsname buf;
    if (uname(&buf) == -1) {
        std::cerr << "Failed to fetch kernel version." << std::endl;
        return "";
    }
    return std::string("/lib/modules/") + buf.release + "/";
}

int main()
{
    std::map<std::string, std::string> fileMap;
    std::string line, filepath, hash;

    // Read the input file with the format <filepath>=<xh32sum>
    std::ifstream inputFile("/etc/version_list.conf");
    while (std::getline(inputFile, line))
    {
        std::stringstream ss(line);
        std::getline(ss, filepath, '=');
        std::getline(ss, hash);
        fileMap[filepath] = hash;
    }
    inputFile.close();

    std::vector<std::string> deviatingFiles;

    // Check xxh32sum for each file and compare
    for (const auto &pair : fileMap) {
        std::string computedHash;

        if (pair.first == "kernel version") {
            struct utsname buf;
            if (uname(&buf) != -1) {
                computedHash = buf.release; // 'release' contains the kernel version
            } else {
                std::cerr << "Failed to fetch kernel version." << std::endl;
                continue; // skip to next iteration
            }
        } else if (pair.first.rfind("kernel module:", 0) == 0) {
            std::string moduleName = pair.first.substr(strlen("kernel module:"));
            std::string modulePath = getKernelModuleBasePath() + moduleName;
            computedHash = getKernelModuleVersion(modulePath);
        } else {
            computedHash = compute_xxh32sum(pair.first);
        }

        if (computedHash != pair.second) {
            deviatingFiles.push_back(pair.first + " (Expected: " + pair.second + ", Actual: " + computedHash + ")");
        }
    }

    // Output the deviating files in JSON format
    std::cout << "[";
    for (size_t i = 0; i < deviatingFiles.size(); ++i) {
        std::cout << "\"" << deviatingFiles[i] << "\"";
        if (i < deviatingFiles.size() - 1) { // if not the last element
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;

    return 0;
}