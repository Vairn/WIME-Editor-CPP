#pragma once
#include <string>
#include <vector>
#include <deque>
#include <functional>

class ConsoleWindow {
public:
    ConsoleWindow();
    ~ConsoleWindow();
    
    void Render();
    
    // Console functionality
    void AddMessage(const std::string& message);
    void AddError(const std::string& error);
    void AddWarning(const std::string& warning);
    void Clear();
    
    // Command handling
    void SetCommandCallback(std::function<void(const std::string&)> callback);
    
private:
    std::deque<std::string> messages;
    char inputBuffer[256];
    std::function<void(const std::string&)> commandCallback;
    bool scrollToBottom;
    
    void RenderMessages();
    void RenderInput();
    void ExecuteCommand(const std::string& command);
    void AddMessageWithColor(const std::string& message, const char* color);
}; 