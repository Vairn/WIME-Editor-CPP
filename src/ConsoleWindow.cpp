#include "ConsoleWindow.h"
#include <imgui.h>
#include <algorithm>
#include <iostream>

ConsoleWindow::ConsoleWindow() 
    : scrollToBottom(true) {
    inputBuffer[0] = '\0'; // Initialize empty string
    AddMessage("WIME Editor C++ Console Ready");
}

ConsoleWindow::~ConsoleWindow() = default;

void ConsoleWindow::Render() {
    if (ImGui::Begin("Console", nullptr)) {
        RenderMessages();
        RenderInput();
    }
    ImGui::End();
}

void ConsoleWindow::AddMessage(const std::string& message) {
    messages.push_back(message);
    if (messages.size() > 1000) { // Limit to 1000 messages
        messages.pop_front();
    }
    scrollToBottom = true;
    std::cout << "ConsoleWindow::AddMessage: " << message << std::endl;
}

void ConsoleWindow::AddError(const std::string& error) {
    AddMessageWithColor("ERROR: " + error, "#FF4444");
}

void ConsoleWindow::AddWarning(const std::string& warning) {
    AddMessageWithColor("WARNING: " + warning, "#FFAA00");
}

void ConsoleWindow::Clear() {
    messages.clear();
    AddMessage("Console cleared");
}

void ConsoleWindow::SetCommandCallback(std::function<void(const std::string&)> callback) {
    commandCallback = callback;
}

void ConsoleWindow::RenderMessages() {
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    for (const auto& message : messages) {
        ImGui::TextWrapped("%s", message.c_str());
    }
    
    if (scrollToBottom) {
        ImGui::SetScrollHereY(1.0f);
        scrollToBottom = false;
    }
    
    ImGui::EndChild();
}

void ConsoleWindow::RenderInput() {
    ImGui::PushItemWidth(-1);
    
    if (ImGui::InputText("##CommandInput", inputBuffer, sizeof(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (inputBuffer[0] != '\0') {
            ExecuteCommand(inputBuffer);
            inputBuffer[0] = '\0';
        }
    }
    
    ImGui::PopItemWidth();
}

void ConsoleWindow::ExecuteCommand(const std::string& command) {
    AddMessage("> " + command);
    
    if (commandCallback) {
        commandCallback(command);
    } else {
        AddMessage("Command callback not set");
    }
}

void ConsoleWindow::AddMessageWithColor(const std::string& message, const char* color) {
    // For now, just add the message normally
    // TODO: Implement colored text when ImGui supports it better
    AddMessage(message);
} 