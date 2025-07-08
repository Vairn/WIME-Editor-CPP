#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

#include "include/Game.h"
#include "include/EditorSettings.h"
#include "include/FileDialog.h"

int main() {
#ifdef _WIN32
    // Enable system DPI awareness for automatic scaling
    SetProcessDPIAware();
#endif

    // Initialize GLFW
    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // Set OpenGL version (ImGui default is 3.0+)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "WIME Editor (C++ Port)", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // Enable DPI scaling - combine system awareness with manual scaling
    float scale = 2.0f; // Make text much larger
    io.FontGlobalScale = scale;
    
    // Enable docking
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    ImGui::StyleColorsDark();
    
    // Global state
    bool gameLoaded = false;
    std::unique_ptr<Game> currentGame;
    EditorSettings settings;
    
    // File dialog filters for WIME games
    std::vector<FileDialog::FileFilter> wimeFilters = {
        {"WIME Executables", "*.exe"},
        {"Amiga Executables", "*.prg"},
        {"Apple IIGS Files", "*.sys16"},
        {"All Files", "*"}
    };

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Main menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open Game", "Ctrl+O")) {
                    std::string filePath = FileDialog::OpenFile(window, "Select WIME Game", wimeFilters);
                    if (!filePath.empty()) {
                        currentGame = std::make_unique<Game>();
                        if (currentGame->LoadGame(filePath)) {
                            gameLoaded = true;
                            settings.lastOpenedFile = filePath;
                        } else {
                            currentGame.reset();
                            gameLoaded = false;
                        }
                    }
                }
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    // TODO: Show about dialog
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Create dock space
        ImGui::DockSpace(ImGui::GetID("MyDockSpace"));

        // Game Info Window
        if (ImGui::Begin("Game Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            if (gameLoaded && currentGame) {
                ImGui::Text("Game: %s", currentGame->Name.c_str());
                ImGui::Text("File: %s", currentGame->FilePath.c_str());
                
                // Show detected format
                const char* formatStr = "Unknown";
                switch (currentGame->format) {
                    case GameFormat::PC: formatStr = "PC"; break;
                    case GameFormat::Amiga: formatStr = "Amiga"; break;
                    case GameFormat::AppleIIGS: formatStr = "Apple IIGS"; break;
                    case GameFormat::AtariST: formatStr = "Atari ST"; break;
                    default: break;
                }
                ImGui::Text("Format: %s", formatStr);
            } else {
                ImGui::Text("Status: No game loaded");
                ImGui::Text("Use File -> Open Game to load a WIME game");
            }
        }
        ImGui::End();

        // Resource Browser Window
        if (ImGui::Begin("Resource Browser", nullptr)) {
            if (gameLoaded && currentGame && currentGame->resource) {
                // Resource tabs
                if (ImGui::BeginTabBar("ResourceTabs")) {
                    if (ImGui::BeginTabItem("Characters")) {
                        ImGui::Text("Character Resources");
                        auto charItems = currentGame->resource->GetItemsByType(ResourceType::CHAR);
                        for (const auto& item : charItems) {
                            if (ImGui::Selectable(item.name.c_str())) {
                                // TODO: Open character editor
                            }
                        }
                        ImGui::EndTabItem();
                    }
                    
                    if (ImGui::BeginTabItem("Strings")) {
                        ImGui::Text("String Resources");
                        auto strItems = currentGame->resource->GetItemsByType(ResourceType::CSTR);
                        for (const auto& item : strItems) {
                            if (ImGui::Selectable(item.name.c_str())) {
                                // TODO: Open string editor
                            }
                        }
                        ImGui::EndTabItem();
                    }
                    
                    if (ImGui::BeginTabItem("Fonts")) {
                        ImGui::Text("Font Resources");
                        auto fontItems = currentGame->resource->GetItemsByType(ResourceType::FONT);
                        for (const auto& item : fontItems) {
                            if (ImGui::Selectable(item.name.c_str())) {
                                // TODO: Open font viewer
                            }
                        }
                        ImGui::EndTabItem();
                    }
                    
                    if (ImGui::BeginTabItem("Images")) {
                        ImGui::Text("Image Resources");
                        auto imgItems = currentGame->resource->GetItemsByType(ResourceType::IMAG);
                        for (const auto& item : imgItems) {
                            if (ImGui::Selectable(item.name.c_str())) {
                                // TODO: Open image viewer
                            }
                        }
                        ImGui::EndTabItem();
                    }
                    
                    if (ImGui::BeginTabItem("Maps")) {
                        ImGui::Text("Map Resources");
                        auto mapItems = currentGame->resource->GetItemsByType(ResourceType::MMAP);
                        for (const auto& item : mapItems) {
                            if (ImGui::Selectable(item.name.c_str())) {
                                // TODO: Open map viewer
                            }
                        }
                        ImGui::EndTabItem();
                    }
                    
                    ImGui::EndTabBar();
                }
            } else {
                ImGui::Text("No resources loaded");
            }
        }
        ImGui::End();

        // Properties Window
        if (ImGui::Begin("Properties", nullptr)) {
            ImGui::Text("Resource Properties");
            ImGui::Text("Select a resource to view its properties");
        }
        ImGui::End();

        // Console/Log Window
        if (ImGui::Begin("Console", nullptr)) {
            ImGui::Text("Application Log");
            ImGui::Text("Ready for commands...");
        }
        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
