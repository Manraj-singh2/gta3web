# GTA 3 Browser Port (re3 WebAssembly)

Porting **Grand Theft Auto III** to modern web browsers using the open-source **re3** engine compiled to **WebAssembly (WASM)** via **Emscripten (emsdk)**, rendered through **WebGL 2.0**, and storing game data via **OPFS (Origin Private File System)**.

---

## Table of Contents
- [1. Project Overview & Architecture](#1-project-overview--architecture)
- [2. Workspace Directory Structure](#2-workspace-directory-structure)
- [3. Prerequisites & Dependencies](#3-prerequisites--dependencies)
- [4. Step-by-Step Implementation Guide](#4-step-by-step-implementation-guide)
  - [Step 4.1: Emscripten Toolchain Setup](#step-41-emscripten-toolchain-setup)
  - [Step 4.2: Engine & Render Backend Configuration](#step-42-engine--render-backend-configuration)
  - [Step 4.3: Compiling re3 to WebAssembly](#step-43-compiling-re3-to-webassembly)
  - [Step 4.4: Asset Pipeline & Virtual File System (VFS)](#step-44-asset-pipeline--virtual-file-system-vfs)
  - [Step 4.5: Web Frontend & Runtime Host](#step-45-web-frontend--runtime-host)
  - [Step 4.6: Running the Local Server (COOP/COEP Headers)](#step-46-running-the-local-server-coopcoep-headers)
- [5. Technical Deep Dive & Gotchas](#5-technical-deep-dive--gotchas)
- [6. Roadmap & Next Tasks](#6-roadmap--next-tasks)

---

## 1. Project Overview & Architecture

The browser port translates the desktop C++ code of `re3` into a sandboxed web application that runs directly on the user's graphics card and CPU without cloud streaming.

```
┌────────────────────────────────────────────────────────────────────────┐
│                        Web Browser Environment                         │
│                                                                        │
│   ┌──────────────────────────┐         ┌───────────────────────────┐   │
│   │     Web Frontend UI      │         │     Storage Layer         │   │
│   │  • HTML5 Canvas (WebGL2) │         │  • OPFS (Private FS)      │   │
│   │  • Asset Import Modal    │◄───────►│  • Local Game Directory   │   │
│   │  • Keyboard / Gamepad    │         │  • Save Games & Settings  │   │
│   └─────────────┬────────────┘         └─────────────┬─────────────┘   │
│                 │                                    │                 │
│                 ▼                                    ▼                 │
│   ┌────────────────────────────────────────────────────────────────┐   │
│   │              Emscripten WebAssembly Runtime                    │   │
│   │                                                                │   │
│   │  • re3 Game Engine (C++ compiled to WebAssembly)               │   │
│   │  • librw: OpenGL 3.3 / ES3 backend mapped to WebGL 2.0         │   │
│   │  • OpenAL Audio Backend mapped to Web Audio API                │   │
│   │  • Emscripten VFS (Virtual File System mapping to OPFS)        │   │
│   └────────────────────────────────────────────────────────────────┘   │
└────────────────────────────────────────────────────────────────────────┘
```

### Core Subsystems
| Component | Desktop Implementation | Browser (WebAssembly) Implementation |
| :--- | :--- | :--- |
| **Engine Core** | Native x86 / x64 C++ | WebAssembly (`.wasm`) |
| **Graphics Engine** | `librw` (DirectX 9 / GL3) | `librw` GL3 + SDL2 mapped to **WebGL 2.0** |
| **Window & Input** | SDL2 / Win32 | Emscripten HTML5 SDL2 abstraction (`<canvas>`) |
| **Audio Engine** | OpenAL / Miles Sound System | OpenAL translated to **Web Audio API** |
| **File System** | Direct OS disk I/O | Emscripten Virtual File System + **OPFS / IDBFS** |

---

## 2. Workspace Directory Structure

```
d:\Projects\gta3\
├── emsdk\                     # Emscripten Compiler SDK
├── re3\                       # Source code repository for re3
│   ├── src\                   # re3 engine source files
│   ├── vendor\
│   │   └── librw\             # RenderWare reimplementation (GL3/D3D/PS2)
│   ├── CMakeLists.txt         # Engine build script
│   └── premake5.lua           # Alternative build configuration
├── Grand Theft Auto 3\        # PC game data & assets (working native re3)
│   ├── anim\
│   ├── audio\
│   ├── data\
│   ├── models\
│   ├── skins\
│   ├── txd\
│   └── re3.exe                # Verified native executable
└── README.md                  # This documentation file
```

---

## 3. Prerequisites & Dependencies

1. **Python & Node.js**: Installed and accessible (installed via `emsdk`).
2. **CMake**: Version 3.16 or newer.
3. **Compiler**: Emscripten SDK (`emsdk`).
4. **Legitimate GTA III PC Assets**: Required for game audio, textures, collision, and map files.

---

## 4. Step-by-Step Implementation Guide

### Step 4.1: Emscripten Toolchain Setup

1. Open PowerShell and navigate to the `emsdk` directory:
   ```powershell
   cd d:\Projects\gta3\emsdk
   ```
2. Install and activate a stable Emscripten version (e.g. `3.1.74` or `latest`):
   ```powershell
   .\emsdk.bat install 3.1.74
   .\emsdk.bat activate 3.1.74
   ```
3. Initialize the environment variables for your current terminal session:
   ```powershell
   .\emsdk_env.bat
   ```
4. Verify installation:
   ```powershell
   emcc -v
   emcmake --version
   ```

---

### Step 4.2: Engine & Render Backend Configuration

`re3` relies on `librw` to render graphics. For WebAssembly:
1. Target **GL3 with SDL2**:
   - `LIBRW_PLATFORM=GL3`
   - `LIBRW_GL3_GFXLIB=SDL2`
2. Target **OpenAL** for audio:
   - `RE3_AUDIO=OAL`
3. Compiler & Linker Settings for Emscripten:
   - `-sUSE_SDL=2`: Enables Emscripten's built-in SDL2 port.
   - `-sUSE_OPENAL=1`: Translates OpenAL calls to Web Audio.
   - `-sMAX_WEBGL_VERSION=2 -sMIN_WEBGL_VERSION=2`: Enforces WebGL 2.0 context.
   - `-sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=256MB`: Allocates sufficient memory for game heap.
   - `-sFETCH=1`: Allows asynchronous network/file fetching.

---

### Step 4.3: Compiling re3 to WebAssembly

Create a dedicated web build directory and configure CMake using `emcmake`:

```powershell
cd d:\Projects\gta3\re3
mkdir build-web
cd build-web

emcmake cmake .. `
  -DCMAKE_BUILD_TYPE=Release `
  -DLIBRW_PLATFORM=GL3 `
  -DLIBRW_GL3_GFXLIB=SDL2 `
  -DRE3_AUDIO=OAL `
  -DCMAKE_CXX_FLAGS="-sUSE_SDL=2 -sUSE_OPENAL=1 -sMAX_WEBGL_VERSION=2 -sMIN_WEBGL_VERSION=2" `
  -DCMAKE_EXE_LINKER_FLAGS="-sUSE_SDL=2 -sUSE_OPENAL=1 -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=256MB -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','FS']"

cmake --build . --config Release
```

The build produces:
- `re3.wasm`: The compiled WebAssembly binary.
- `re3.js`: The Emscripten JavaScript runtime glue.

---

### Step 4.4: Asset Pipeline & Virtual File System (VFS)

Because Grand Theft Auto III game assets total between 800 MB and 1.2 GB, they cannot be bundled directly into the `.wasm` file. Instead:

1. **Packaging**:
   Package the contents of `Grand Theft Auto 3\` into a compressed tarball or zip file (`game.tar.gz`):
   ```
   game.tar.gz
   ├── anim/
   ├── audio/
   ├── data/
   ├── models/
   ├── skins/
   ├── TEXT/
   └── txd/
   ```

2. **Client-Side Storage (OPFS / Origin Private File System)**:
   - When the user visits the webpage for the first time, provide a file picker to select `game.tar.gz` (or use the Directory Picker API to select the local `Grand Theft Auto 3` folder).
   - The browser extracts the files directly into **OPFS** using a web worker.
   - On subsequent visits, the game mounts OPFS immediately with zero upload or download time.

---

### Step 4.5: Web Frontend & Runtime Host

The web interface requires:
1. **HTML5 `<canvas id="canvas">`**: Captures WebGL2 context, keyboard, pointer lock, and gamepad inputs.
2. **Emscripten Module Config**:
   ```javascript
   var Module = {
       canvas: document.getElementById('canvas'),
       preRun: [function() {
           // Mount OPFS or IDBFS into Emscripten virtual root '/'
       }],
       postRun: [],
       print: function(text) { console.log(text); },
       printErr: function(text) { console.error(text); }
   };
   ```
3. **Controls & Audio**:
   - Pointer Lock: `canvas.requestPointerLock()` for camera control.
   - Web Audio Context Resume: User interaction is required before playing game audio due to browser autoplay policies.

---

### Step 4.6: Running the Local Server (COOP/COEP Headers)

For multithreading (SharedArrayBuffer) and high-performance browser timers, the web server must return Cross-Origin Isolation headers:

```http
Cross-Origin-Opener-Policy: same-origin
Cross-Origin-Embedder-Policy: require-corp
```

#### Running a quick local server:
Using Node.js:
```bash
npx serve -s . --cors
```
Or with custom headers in Python:
```python
# server.py
from http.server import HTTPServer, SimpleHTTPRequestHandler

class CORSRequestHandler(SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        super().end_headers()

HTTPServer(('127.0.0.1', 8080), CORSRequestHandler).serve_forever()
```

---

## 5. Technical Deep Dive & Gotchas

1. **Blocking Main Loop**:
   Desktop game engines use a continuous loop:
   ```cpp
   while (isRunning) {
       ProcessEvents();
       Update();
       Render();
   }
   ```
   In the browser, a blocking loop freezes the webpage. Emscripten handles this using either:
   - `emscripten_set_main_loop(GameLoopCallback, 0, 1);` (Preferred for performance)
   - `-sASYNCIFY=1` (Allows blocking calls but incurs a binary size and CPU overhead)

2. **Case Sensitivity in File Paths**:
   Windows file systems are case-insensitive (`models\gta3.img` == `MODELS\GTA3.IMG`), but Emscripten's virtual file systems and tar archives are case-sensitive. Filenames should be normalized to lowercase or matched using case-insensitive path hooks.

3. **Memory Allocation**:
   GTA III streaming requires sufficient virtual memory. Always enable `-sALLOW_MEMORY_GROWTH=1` and set `-sINITIAL_MEMORY=256MB` or higher.

4. **Audio Format**:
   Original game radio stations and SFX are encoded in WAV, MP3, or custom audio archives. Ensure the audio files are present in the virtual `/audio` directory.

---

## 6. Roadmap & Next Tasks

- [ ] **Task 1: Activate Emscripten**: Complete `emsdk install 3.1.74` and activate environment in terminal.
- [ ] **Task 2: CMake Web Toolchain**: Create a custom CMake profile / preset targeting Emscripten with GL3 + SDL2.
- [ ] **Task 3: Main Loop Adapter**: Check re3's platform main loop and hook `emscripten_set_main_loop`.
- [ ] **Task 4: Build Web Artifacts**: Compile `re3.wasm` and `re3.js`.
- [ ] **Task 5: Frontend Web Player**: Build `index.html`, `style.css`, and OPFS asset loader.
- [ ] **Task 6: Browser Testing**: Test game initialization, menu screens, audio playback, and in-game rendering.
