#pragma once
#include <string>
#include <vector>
#include <GLFW/glfw3.h>

class FileDialog {
public:
    struct FileFilter {
        std::string name;
        std::string pattern;
    };

    static std::string OpenFile(GLFWwindow* window, const std::string& title, 
                               const std::vector<FileFilter>& filters = {});
    
    static std::string SaveFile(GLFWwindow* window, const std::string& title,
                               const std::vector<FileFilter>& filters = {});

private:
    static std::string GetFilterString(const std::vector<FileFilter>& filters);
}; 