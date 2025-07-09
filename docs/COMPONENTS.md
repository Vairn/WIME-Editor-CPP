# WIME Editor C++ Component Documentation

## Core Components

### 1. BinaryFile Class

**File:** `include/BinaryFile.h`, `src/BinaryFile.cpp`

**Purpose:** Provides cross-platform binary file I/O with endianness support for reading WIME game files.

#### Class Interface

```cpp
class BinaryFile {
public:
    // Construction and destruction
    BinaryFile(const std::string& filename);
    ~BinaryFile();
    
    // File operations
    bool IsOpen() const;
    std::string GetFilename() const;
    size_t GetPosition();
    void SetPosition(size_t position);
    size_t GetLength();
    
    // Byte operations
    uint8_t ReadByteUnsigned();
    int8_t ReadByteSigned();
    void WriteByteUnsigned(uint8_t value);
    void WriteByteSigned(int8_t value);
    
    // Word operations (16-bit)
    int16_t ReadWordSigned(Endianness endian = Endianness::Little);
    uint16_t ReadWordUnsigned(Endianness endian = Endianness::Little);
    void WriteWordSigned(int16_t value, Endianness endian = Endianness::Little);
    void WriteWordUnsigned(uint16_t value, Endianness endian = Endianness::Little);
    
    // Longword operations (32-bit)
    int32_t ReadLongwordSigned(Endianness endian = Endianness::Little);
    uint32_t ReadLongwordUnsigned(Endianness endian = Endianness::Little);
    void WriteLongwordSigned(int32_t value, Endianness endian = Endianness::Little);
    void WriteLongwordUnsigned(uint32_t value, Endianness endian = Endianness::Little);
    
    // String operations
    std::string ReadString(size_t length);
    void WriteString(const std::string& value);
    
    // Array operations
    std::vector<uint8_t> ReadBytes(size_t count);
    void WriteBytes(const std::vector<uint8_t>& data);
    
    // Utility functions
    static void SwapWord(int16_t& word);
    static void SwapLongword(int32_t& longword);
    static void Nibbler(uint16_t value, uint8_t& byte1, uint8_t& byte2);
    static int16_t ReadShort(uint8_t byte1, uint8_t byte2, Endianness endian = Endianness::Little);
};
```

#### Usage Examples

```cpp
// Reading a WIME resource file
BinaryFile file("game.res");
if (file.IsOpen()) {
    // Read header with big-endian format (Amiga)
    uint32_t size = file.ReadLongwordUnsigned(Endianness::Big);
    uint32_t dataSize = file.ReadLongwordUnsigned(Endianness::Big);
    
    // Read string data
    std::string name = file.ReadString(16);
    
    // Read array of bytes
    std::vector<uint8_t> data = file.ReadBytes(1024);
}
```

#### Error Handling

The class throws exceptions for:
- File open failures
- End-of-file conditions
- Invalid position access
- Write failures

```cpp
try {
    BinaryFile file("nonexistent.res");
    uint32_t value = file.ReadLongwordUnsigned();
} catch (const std::runtime_error& e) {
    // Handle file errors
}
```

### 2. Game Class

**File:** `include/Game.h`, `src/Game.cpp`

**Purpose:** Central game data management, format detection, and resource loading coordination.

#### Class Interface

```cpp
class Game {
public:
    // Public data members
    std::string Name;
    std::string FilePath;
    bool IsLoaded;
    GameFormat format;
    std::unique_ptr<ResourceIndex> resource;
    std::unique_ptr<FileFormat> fileFormat;
    
    // Public methods
    Game();
    bool LoadGame(const std::string& filePath);
    void UnloadGame();
    
private:
    // Internal implementation
    bool InitializeGameData(const std::string& filePath);
    GameFormat DetectFormat(const std::string& filePath, BinaryFile& file);
    void LoadRealResources(const std::string& gamePath);
};
```

#### Game Format Detection

```cpp
enum class GameFormat {
    Unknown,
    PC,        // DOS/Windows version
    Amiga,     // Amiga version
    AppleIIGS, // Apple IIGS version
    AtariST    // Atari ST version
};
```

#### Usage Examples

```cpp
// Loading a game
Game game;
if (game.LoadGame("WIME-DOS/START.EXE")) {
    std::cout << "Loaded: " << game.Name << std::endl;
    std::cout << "Format: " << (game.format == GameFormat::PC ? "PC" : "Other") << std::endl;
    
    // Access resources
    if (game.resource) {
        auto characters = game.resource->GetItemsByType(ResourceType::CHAR);
        std::cout << "Found " << characters.size() << " characters" << std::endl;
    }
}
```

### 3. ResourceLoader Class

**File:** `include/ResourceLoader.h`, `src/ResourceLoader.cpp`

**Purpose:** Parses WIME resource files (.res) and extracts resource metadata and data.

#### Data Structures

```cpp
// Resource file header
struct ResourceHeader {
    uint32_t size;
    uint32_t dataSegmentSize;
    uint32_t dataSize;
    uint32_t fileEndLength;
};

// Resource map entry
struct ResourceMap {
    uint16_t number;
    uint32_t offset;
    uint16_t multiplier;
};

// Resource identifier
struct ResourceIdentifier {
    std::string resourceID;
    uint16_t resourceQTY;
};
```

#### Class Interface

```cpp
class ResourceLoader {
public:
    static std::unique_ptr<ResourceIndex> LoadResourceFile(
        const std::string& filename, Endianness endian);
    
private:
    // Internal parsing methods
    static ResourceHeader ReadResourceHeader(BinaryFile& file, Endianness endian);
    static std::vector<ResourceIdentifier> ReadResourceIdentifiers(
        BinaryFile& file, uint32_t filePointer, Endianness endian);
```

### 4. ResourceViewers System

**Files:** `include/ResourceViewers.h`, `src/ResourceViewers.cpp`

**Purpose:** Provides specialized viewers for different resource types, enabling type-specific rendering and editing capabilities.

#### Architecture

The ResourceViewers system uses a polymorphic design with a base `ResourceViewer` class and specialized implementations for each resource type:

```cpp
// Base class for all resource viewers
class ResourceViewer {
public:
    virtual ~ResourceViewer() = default;
    virtual void Render() = 0;
    virtual void SetResource(const std::shared_ptr<ResourceItem>& resource) = 0;
    virtual void SetGameFilePath(const std::string& filePath) = 0;
    virtual void ClearCache() = 0;
};
```

#### Implemented Viewers

**StringResourceViewer (CSTR)**
- Displays string content with character analysis
- Shows printable characters and hex codes for non-printable
- Handles newline conversion (byte 10 → '\n')

**MapResourceViewer (MMAP)**
- Decompresses ByteRun-encoded map data
- Displays map properties (2560x1584 pixels, 160x99 tile grid)
- Shows decompressed map data as a preview grid
- Handles MMAP-specific data offsets (offset+8, size-18)

**BinaryResourceViewer (Generic)**
- Provides hex dump for binary resources
- Shows ASCII representation alongside hex values
- Handles all other resource types (CHAR, FONT, FRML, IMAG, etc.)

#### Factory Function

```cpp
std::unique_ptr<ResourceViewer> CreateResourceViewer(ResourceType type);
```

Creates the appropriate viewer based on resource type:
- `ResourceType::CSTR` → `StringResourceViewer`
- `ResourceType::MMAP` → `MapResourceViewer`
- All others → `BinaryResourceViewer`

#### Usage Examples

```cpp
// Create a viewer for a string resource
auto viewer = CreateResourceViewer(ResourceType::CSTR);
viewer->SetResource(resourceItem);
viewer->SetGameFilePath(gamePath);
viewer->Render(); // Displays string content with analysis

// Create a viewer for a map resource
auto mapViewer = CreateResourceViewer(ResourceType::MMAP);
mapViewer->SetResource(mapResource);
mapViewer->Render(); // Shows decompressed map grid
```

### 5. PreviewWindow Class

**Files:** `include/PreviewWindow.h`, `src/PreviewWindow.cpp`

**Purpose:** Provides a dedicated preview window for viewing and editing resource content, separate from the properties display.

#### Features

- **Resource-Specific Content**: Shows actual resource data (strings, maps, etc.) rather than just metadata
- **String Editing**: CSTR resources display in an editable multiline text box
- **Map Preview**: MMAP resources show decompressed map data as a grid
- **Independent Windows**: Each preview window is self-contained and can be docked/moved
- **Shared Instance**: Integrated into main UI as a shared window (like Properties)

#### Class Interface

```cpp
class PreviewWindow {
public:
    PreviewWindow();
    void SetResource(const std::shared_ptr<ResourceItem>& resource, 
                    const std::string& gameFilePath);
    void Render();
    bool IsOpen() const;
    void Close();
    std::string GetTitle() const;
};
```

#### String Editing Features

- **Multiline Text Box**: Large editing area for string content
- **Dirty State Tracking**: Shows when content has been modified
- **Save Functionality**: Placeholder for writing changes back to file
- **Newline Handling**: Converts byte 10 to '\n' for display

#### Map Preview Features

- **Decompression**: Handles ByteRun decompression of map data
- **Grid Display**: Shows map data as a 160x99 tile grid
- **Property Display**: Shows map dimensions, tile size, etc.
- **Preview Limiting**: Shows first 10 rows, 20 columns for performance

#### Usage

```cpp
// Create and configure preview window
auto preview = std::make_unique<PreviewWindow>();
preview->SetResource(resourceItem, gameFilePath);

// Render the preview (called each frame)
preview->Render();

// Check if window is still open
if (!preview->IsOpen()) {
    // Window was closed by user
}
```

#### Integration with Main UI

The PreviewWindow is integrated into the main EditorUI:
- Accessible via View → Preview menu
- Updates automatically when resource selection changes
- Can be docked, moved, and resized like other windows
- Shares the same resource selection as the Properties window
    static std::vector<ResourceMap> ReadResourceMaps(
        BinaryFile& file, uint32_t keyPosition, uint16_t count, Endianness endian);
    
    // Utility methods
    static uint32_t GetResourceKeyPosition(BinaryFile& file, Endianness endian);
    static std::string GetChunkID(BinaryFile& file, uint32_t offset, Endianness endian);
    static uint16_t GetChunkQTY(BinaryFile& file, uint32_t offset, Endianness endian);
    static ResourceType GetResourceType(const std::string& resourceID);
    static bool ValidateResourceHeader(const std::string& filename, Endianness endian);
};
```

#### Usage Examples

```cpp
// Loading a resource file
auto resourceIndex = ResourceLoader::LoadResourceFile("AANIMS.RES", Endianness::Little);
if (resourceIndex) {
    std::cout << "Resource ID: " << resourceIndex->ID << std::endl;
    
    // Get all character resources
    auto chars = resourceIndex->GetItemsByType(ResourceType::CHAR);
    for (const auto& item : chars) {
        std::cout << "Character: " << item.name 
                  << " (offset: " << item.offset 
                  << ", size: " << item.size << ")" << std::endl;
    }
}
```

### 4. ResourceIndex Class

**File:** `include/ResourceIndex.h`

**Purpose:** Organizes and indexes game resources by type and provides query interfaces.

#### Resource Types

```cpp
enum class ResourceType {
    CHAR,   // Character data and sprites
    CSTR,   // String resources
    FONT,   // Font definitions
    FRML,   // Form/UI elements
    IMAG,   // Image resources
    MMAP,   // Map data
    ARCHIVE // Archive files
};
```

#### Data Structure

```cpp
struct ResourceItem {
    std::string name;
    uint32_t offset;
    uint32_t size;
    ResourceType type;
    
    ResourceItem(const std::string& n = "", uint32_t off = 0, 
                 uint32_t sz = 0, ResourceType t = ResourceType::CHAR)
        : name(n), offset(off), size(sz), type(t) {}
};
```

#### Class Interface

```cpp
class ResourceIndex {
public:
    std::string ID;
    std::vector<ResourceItem> items;
    
    // Construction
    ResourceIndex();
    ResourceIndex(const std::string& id);
    
    // Resource management
    void AddItem(const std::string& name, uint32_t offset, 
                 uint32_t size, ResourceType type);
    
    // Query methods
    std::vector<ResourceItem> GetItemsByType(ResourceType type) const;
    size_t GetItemCount(ResourceType type) const;
};
```

#### Usage Examples

```cpp
// Creating and populating a resource index
ResourceIndex index("AANIMS");
index.AddItem("GANDALF", 0x1000, 1024, ResourceType::CHAR);
index.AddItem("ARAGORN", 0x2000, 2048, ResourceType::CHAR);
index.AddItem("MAIN_FONT", 0x3000, 512, ResourceType::FONT);

// Querying resources
auto characters = index.GetItemsByType(ResourceType::CHAR);
std::cout << "Found " << characters.size() << " characters" << std::endl;

auto fontCount = index.GetItemCount(ResourceType::FONT);
std::cout << "Found " << fontCount << " fonts" << std::endl;
```

### 5. FileDialog Class

**File:** `include/FileDialog.h`, `src/FileDialog.cpp`

**Purpose:** Provides cross-platform native file dialogs with GLFW integration.

#### Data Structures

```cpp
struct FileFilter {
    std::string name;    // Display name
    std::string pattern; // File pattern (e.g., "*.exe")
};
```

#### Class Interface

```cpp
class FileDialog {
public:
    static std::string OpenFile(GLFWwindow* window, const std::string& title, 
                               const std::vector<FileFilter>& filters = {});
    
    static std::string SaveFile(GLFWwindow* window, const std::string& title,
                               const std::vector<FileFilter>& filters = {});
    
private:
    static std::string GetFilterString(const std::vector<FileFilter>& filters);
};
```

#### Usage Examples

```cpp
// Setting up file filters for WIME games
std::vector<FileDialog::FileFilter> wimeFilters = {
    {"WIME Executables", "*.exe"},
    {"Amiga Executables", "*.prg"},
    {"Apple IIGS Files", "*.sys16"},
    {"All Files", "*"}
};

// Opening a file dialog
std::string filePath = FileDialog::OpenFile(window, "Select WIME Game", wimeFilters);
if (!filePath.empty()) {
    std::cout << "Selected: " << filePath << std::endl;
}

// Saving a file
std::string savePath = FileDialog::SaveFile(window, "Save Resource", 
    {{"Resource Files", "*.res"}});
```

### 6. EditorSettings Class

**File:** `include/EditorSettings.h`, `src/EditorSettings.cpp`

**Purpose:** Manages application settings and user preferences.

#### Class Interface

```cpp
class EditorSettings {
public:
    std::string wimeDIRECTORY;
    std::string lastOpenedFile;
    bool autoSave;
    bool showDebugInfo;
    int windowWidth;
    int windowHeight;
    bool windowMaximized;
    
    EditorSettings();
};
```

#### Usage Examples

```cpp
// Creating settings
EditorSettings settings;
settings.windowWidth = 1280;
settings.windowHeight = 720;
settings.lastOpenedFile = "WIME-DOS/START.EXE";
settings.autoSave = true;

// Using settings in the application
if (!settings.lastOpenedFile.empty()) {
    // Auto-load last opened file
    game.LoadGame(settings.lastOpenedFile);
}
```

### 7. FileFormat Class

**File:** `include/FileFormat.h`

**Purpose:** Defines game format specifications and metadata.

#### Class Interface

```cpp
class FileFormat {
public:
    std::string Name;
    std::string Endian;
    std::string DataEndian;
    std::string ExecutableFile;
    std::string Icon;
    int BitPlanes;
    int FRMLBitplanes;
    
    FileFormat();
    FileFormat(const std::string& name, const std::string& endian, 
               const std::string& dataEndian, const std::string& executableFile,
               const std::string& icon, int bitPlanes, int frmlBitplanes);
};
```

#### Usage Examples

```cpp
// Creating format definitions
FileFormat pcFormat("PC", "Little", "Little", "START.EXE", "pc_icon.png", 4, 4);
FileFormat amigaFormat("Amiga", "Big", "Big", "WIME", "amiga_icon.png", 5, 5);

// Using format information
if (game.fileFormat) {
    std::cout << "Format: " << game.fileFormat->Name << std::endl;
    std::cout << "Endianness: " << game.fileFormat->Endian << std::endl;
    std::cout << "Bit Planes: " << game.fileFormat->BitPlanes << std::endl;
}
```

## Component Interactions

### Typical Workflow

1. **Application Startup**
   ```cpp
   EditorSettings settings;  // Load settings
   Game game;               // Initialize game manager
   ```

2. **File Selection**
   ```cpp
   std::string filePath = FileDialog::OpenFile(window, "Select Game", filters);
   ```

3. **Game Loading**
   ```cpp
   if (game.LoadGame(filePath)) {
       settings.lastOpenedFile = filePath;  // Remember selection
   }
   ```

4. **Resource Access**
   ```cpp
   if (game.resource) {
       auto items = game.resource->GetItemsByType(ResourceType::CHAR);
       // Process resources...
   }
   ```

### Error Handling Patterns

```cpp
// File operations
try {
    BinaryFile file(filename);
    // Process file...
} catch (const std::runtime_error& e) {
    // Handle file errors
}

// Resource loading
auto resource = ResourceLoader::LoadResourceFile(filename, endian);
if (!resource) {
    // Handle loading failure
}

// Game loading
if (!game.LoadGame(filePath)) {
    // Handle game loading failure
}
```

## Performance Considerations

### Memory Management
- Use smart pointers for automatic cleanup
- Minimize data copying with references
- Use RAII for resource management

### File I/O Optimization
- Read data in chunks when possible
- Cache frequently accessed resources
- Use efficient data structures

### UI Responsiveness
- Load resources asynchronously (future)
- Update UI incrementally
- Provide progress feedback 