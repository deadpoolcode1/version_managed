#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <xxhash.h>
#include <sys/utsname.h>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <set>
#include <cstdlib> // Include this for system()
#include <cstdio>  // Include this for popen()

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
    std::string cmd = "/sbin/modinfo -F vermagic " + modulePath + " 2>/dev/null";
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        std::cerr << "Failed to run modinfo command." << std::endl;
        return "";
    }

    char buffer[128];
    std::string result = "";

    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }

    pclose(pipe);

    // Parsing the version from the modinfo output
    size_t spacePos = result.find(' ');
    if (spacePos != std::string::npos)
    {
        result = result.substr(0, spacePos);
    }
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());

    return result;
}

std::string getKernelModuleBasePath()
{
    struct utsname buf;
    if (uname(&buf) == -1)
    {
        std::cerr << "Failed to fetch kernel version." << std::endl;
        return "";
    }
    return std::string("/lib/modules/") + buf.release + "/";
}

std::string trim(const std::string &str)
{
    auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
    auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
    return (end <= start ? std::string() : std::string(start, end));
}

int main()
{
    std::string line, filepath, hash;
    std::map<std::string, std::string> fileMap;
    std::set<std::string> checkedModules; // This set keeps track of checked modules
    std::vector<std::string> deviatingFiles;

    // Read the input file with the format <filepath>=<xh32sum>
    std::ifstream inputFile("/etc/release_list.conf");
    if (!inputFile.is_open())
    {
        std::cerr << "Failed to open /etc/release_list.conf" << std::endl;
        return 1;
    }

    while (std::getline(inputFile, line))
    {
        std::stringstream ss(line);
        std::getline(ss, filepath, '=');
        std::getline(ss, hash);
        fileMap[filepath] = hash;
    }
    inputFile.close();

    for (const auto &pair : fileMap)
    {
        std::string computedHash;

        if (pair.first == "kernel version")
        {
            struct utsname buf;
            if (uname(&buf) != -1)
            {
                computedHash = buf.release; // 'release' contains the kernel version
            }
            else
            {
                std::cerr << "Failed to fetch kernel version." << std::endl;
                continue; // skip to the next iteration
            }
        }
        else if (pair.first.rfind("kernel module:", 0) == 0)
        {
            std::string moduleName = pair.first.substr(strlen("kernel module:"));
            std::string modulePath = getKernelModuleBasePath() + moduleName;

            std::string versionFromModinfo = getKernelModuleVersion(modulePath);
            versionFromModinfo = trim(versionFromModinfo);
            std::string trimmedExpected = trim(pair.second);

            if (versionFromModinfo != trimmedExpected)
            {
                deviatingFiles.push_back(pair.first + " (Expected: " + trimmedExpected + ", Actual: " + versionFromModinfo + ")");
            }

            checkedModules.insert(pair.first); // Mark this module as checked
        }
        else
        {
            computedHash = compute_xxh32sum(pair.first);
        }

        if (checkedModules.find(pair.first) == checkedModules.end() && computedHash != pair.second)
        {
            deviatingFiles.push_back(pair.first + " (Expected: " + pair.second + ", Actual: " + computedHash + ")");
        }
    }

    // Output the deviating files in JSON format
    std::cout << "[";
    for (size_t i = 0; i < deviatingFiles.size(); ++i)
    {
        std::cout << "\"" << deviatingFiles[i] << "\"";
        if (i < deviatingFiles.size() - 1)
        { // if not the last element
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;

    return 0;
}
