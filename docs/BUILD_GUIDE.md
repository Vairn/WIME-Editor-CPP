# WIME Editor C++ Build Guide

## Prerequisites

### Required Software
- **CMake 3.15 or higher**
- **C++20 compatible compiler**
- **Git** (for dependency management)
- **OpenGL development libraries**

### Platform-Specific Requirements

#### Windows
- **Visual Studio 2019/2022** or **MinGW-w64**
- **Windows SDK** (for native file dialogs)
- **OpenGL32** (usually included with Windows)

#### Linux
- **GCC 10+** or **Clang 12+**
- **OpenGL development packages**:
  ```bash
  # Ubuntu/Debian
  sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev
  
  # Fedora/RHEL
  sudo dnf install mesa-libGL-devel mesa-libGLU-devel
  
  # Arch Linux
  sudo pacman -S mesa
  ```

#### macOS
- **Xcode Command Line Tools** or **Xcode**
- **OpenGL** (included with macOS)
- **Homebrew** (recommended for development tools)

## Dependencies

The project uses CPM.cmake for automatic dependency management:

### Core Dependencies
- **GLFW 3.3.9**: Cross-platform window management
- **ImGui (docking branch)**: Immediate mode GUI
- **portable-file-dialogs**: Cross-platform native file dialogs

### Dependency Details

#### GLFW
- **Version**: 3.3.9 (specific commit for stability)
- **Purpose**: Window creation, OpenGL context, input handling
- **Features**: Cross-platform, lightweight, well-maintained

#### ImGui
- **Branch**: docking (for advanced docking features)
- **Purpose**: User interface rendering
- **Features**: Immediate mode, OpenGL backend, DPI scaling

#### portable-file-dialogs
- **Type**: Single-header library
- **Purpose**: Native file dialogs
- **Platforms**: Windows (Win32), Linux (GTK+/KDE), macOS (Cocoa)

## Building the Project

### Step 1: Clone and Prepare
```bash
# Clone the repository
git clone <repository-url>
cd WIME-Editor-CPP

# Create build directory
mkdir build
cd build
```

### Step 2: Configure with CMake
```bash
# Basic configuration
cmake ..

# Or with specific generator
cmake -G "Visual Studio 16 2019" ..  # Windows
cmake -G "Unix Makefiles" ..          # Linux/macOS
```

### Step 3: Build the Project
```bash
# Build all targets
cmake --build .

# Or build specific target
cmake --build . --target WIMEEditorCPP

# Parallel build (recommended)
cmake --build . --parallel
```

### Step 4: Run the Application
```bash
# Run the executable
./WIMEEditorCPP  # Linux/macOS
WIMEEditorCPP.exe  # Windows
```

## Platform-Specific Build Instructions

### Windows

#### Using Visual Studio
```bash
# Generate Visual Studio solution
cmake -G "Visual Studio 16 2019" -A x64 ..

# Build with Visual Studio
cmake --build . --config Release
```

#### Using MinGW-w64
```bash
# Install MinGW-w64 (if not already installed)
# Download from: https://www.mingw-w64.org/

# Configure with MinGW
cmake -G "MinGW Makefiles" ..

# Build
cmake --build .
```

#### Troubleshooting Windows Issues
- **OpenGL not found**: Ensure Windows SDK is installed
- **DLL errors**: Copy required DLLs to executable directory
- **DPI issues**: Application handles DPI awareness automatically

### Linux

#### Ubuntu/Debian
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install build-essential cmake git
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### Fedora/RHEL
```bash
# Install dependencies
sudo dnf install gcc-c++ cmake git
sudo dnf install mesa-libGL-devel mesa-libGLU-devel

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### Arch Linux
```bash
# Install dependencies
sudo pacman -S base-devel cmake git mesa

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### Troubleshooting Linux Issues
- **OpenGL not found**: Install mesa development packages
- **File dialog issues**: Ensure GTK+ or KDE is installed
- **Permission errors**: Check file permissions and ownership

### macOS

#### Using Xcode
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Build
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

#### Using Homebrew
```bash
# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake

# Build
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

#### Troubleshooting macOS Issues
- **OpenGL deprecation warnings**: Normal on macOS, functionality works
- **File dialog issues**: Ensure proper permissions
- **Code signing**: May need to sign executable for distribution

## CMake Configuration Options

### Available Options
```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..

# Specify C++ standard
cmake -DCMAKE_CXX_STANDARD=20 ..

# Enable verbose output
cmake --build . --verbose
```

### Custom Configuration
```bash
# Set custom compiler
cmake -DCMAKE_C_COMPILER=gcc-11 -DCMAKE_CXX_COMPILER=g++-11 ..

# Set custom installation directory
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..

# Enable specific features
cmake -DENABLE_DEBUG=ON ..
```

## Development Build

### Debug Configuration
```bash
# Configure for debug
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Build with debug symbols
cmake --build . --config Debug
```

### Development Tools

#### Code Formatting
```bash
# Install clang-format
# Ubuntu: sudo apt-get install clang-format
# macOS: brew install clang-format

# Format code
find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

#### Static Analysis
```bash
# Install cppcheck
# Ubuntu: sudo apt-get install cppcheck
# macOS: brew install cppcheck

# Run static analysis
cppcheck --enable=all --std=c++20 src/ include/
```

## Testing the Build

### Basic Functionality Test
1. **Launch the application**
2. **Test file dialog**: File → Open Game
3. **Test game loading**: Select a WIME game file
4. **Test resource display**: Check resource tabs
5. **Test UI responsiveness**: Resize window, check DPI scaling

### Platform-Specific Tests

#### Windows
- Test with different DPI settings
- Test with different Windows themes
- Test file dialog with various file types

#### Linux
- Test with different desktop environments (GNOME, KDE)
- Test with different display servers (X11, Wayland)
- Test file permissions and access

#### macOS
- Test with different macOS versions
- Test with different display configurations
- Test file dialog permissions

## Troubleshooting Common Issues

### Build Issues

#### CMake Configuration Fails
```bash
# Clear CMake cache
rm -rf build
mkdir build && cd build

# Reconfigure with verbose output
cmake .. --debug-output
```

#### Compilation Errors
```bash
# Check compiler version
g++ --version
clang++ --version

# Ensure C++20 support
echo '#include <version>' | g++ -std=c++20 -x c++ -E -
```

#### Linker Errors
```bash
# Check for missing libraries
ldd ./WIMEEditorCPP  # Linux
otool -L ./WIMEEditorCPP  # macOS
dumpbin /dependents WIMEEditorCPP.exe  # Windows
```

### Runtime Issues

#### Application Won't Start
```bash
# Check for missing dependencies
# Linux: ldd ./WIMEEditorCPP
# macOS: otool -L ./WIMEEditorCPP
# Windows: Dependency Walker

# Check console output for error messages
./WIMEEditorCPP 2>&1 | tee output.log
```

#### File Dialog Issues
- **Windows**: Check Windows SDK installation
- **Linux**: Install GTK+ or KDE development packages
- **macOS**: Check file permissions and sandboxing

#### OpenGL Issues
```bash
# Check OpenGL support
glxinfo | grep "OpenGL version"  # Linux
system_profiler SPDisplaysDataType  # macOS
```

### Performance Issues

#### Slow Build Times
```bash
# Use parallel builds
cmake --build . --parallel

# Use ccache for faster rebuilds
# Ubuntu: sudo apt-get install ccache
# macOS: brew install ccache
```

#### Slow Application Startup
- Check for large resource files
- Monitor memory usage
- Profile with tools like gprof or Valgrind

## Distribution

### Creating Release Builds
```bash
# Configure for release
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build optimized version
cmake --build . --config Release

# Strip debug symbols (Linux/macOS)
strip ./WIMEEditorCPP
```

### Packaging

#### Windows
- Use NSIS or Inno Setup for installer
- Include required Visual C++ redistributables
- Sign executable for Windows SmartScreen

#### Linux
- Create .deb package with CPack
- Include desktop file and icons
- Test on multiple distributions

#### macOS
- Create .dmg package
- Sign with Apple Developer certificate
- Notarize for Gatekeeper compatibility

## Continuous Integration

### GitHub Actions Example
```yaml
name: Build and Test
on: [push, pull_request]
jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
    steps:
    - uses: actions/checkout@v2
    - name: Configure CMake
      run: cmake -B build -DCMAKE_BUILD_TYPE=Release
    - name: Build
      run: cmake --build build --config Release
    - name: Test
      run: ./build/WIMEEditorCPP --help
```

## Support and Resources

### Getting Help
- Check the troubleshooting section above
- Review CMake and compiler documentation
- Search for platform-specific issues

### Useful Tools
- **CMake GUI**: Visual CMake configuration
- **ccmake**: Terminal-based CMake configuration
- **CMakePresets**: Standardized build configurations

### Documentation
- [CMake Documentation](https://cmake.org/documentation/)
- [C++20 Reference](https://en.cppreference.com/)
- [OpenGL Documentation](https://www.opengl.org/documentation/)
- [ImGui Documentation](https://github.com/ocornut/imgui) 