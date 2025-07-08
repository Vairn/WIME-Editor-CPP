#include "FileDialog.h"
#include "portable-file-dialogs.h"
#include <iostream>

std::string FileDialog::OpenFile(GLFWwindow* window, const std::string& title, 
                                const std::vector<FileFilter>& filters) {
    try {
        std::vector<std::string> filterPatterns;
        for (const auto& filter : filters) {
            filterPatterns.push_back(filter.name);    // Display name
            filterPatterns.push_back(filter.pattern); // Pattern
        }
        
        auto result = pfd::open_file(title, "", filterPatterns);
        auto files = result.result();
        if (!files.empty()) {
            return files[0];
        }
    } catch (const std::exception& e) {
        std::cerr << "File dialog error: " << e.what() << std::endl;
    }
    
    return "";
}

std::string FileDialog::SaveFile(GLFWwindow* window, const std::string& title,
                                const std::vector<FileFilter>& filters) {
    try {
        std::vector<std::string> filterPatterns;
        for (const auto& filter : filters) {
            filterPatterns.push_back(filter.name);    // Display name
            filterPatterns.push_back(filter.pattern); // Pattern
        }
        
        auto result = pfd::save_file(title, "", filterPatterns);
        return result.result();
    } catch (const std::exception& e) {
        std::cerr << "Save dialog error: " << e.what() << std::endl;
    }
    
    return "";
}

std::string FileDialog::GetFilterString(const std::vector<FileFilter>& filters) {
    std::string result;
    for (const auto& filter : filters) {
        if (!result.empty()) result += ";";
        result += filter.pattern;
    }
    return result;
} 