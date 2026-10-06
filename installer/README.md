# Media Studio Windows Installer Guide
# Developer: Mon Aquino

This directory contains the scripts and configuration to build a professional Windows installer for Media Studio.

The setup title, component labels, shortcuts, installation directory, and Windows
uninstall entry use Media Studio. Rebuild the installer after changing branding;
renaming the executable alone does not update its embedded text.

The application, updater, installer, and uninstaller use `installer\media.ico`.
Rebuild the application and installer after updating that icon.

Existing Media Studio installations are not migrated or removed by this installer.

Media Studio disables automatic and manual upstream OBS update checks, hides the
update settings, and disables upstream repair. Saved update preferences do not
re-enable the updater. Install newer Media Studio releases using a new Media
Studio installer instead.

## ✅ Installation Complete!

The installer has been successfully created at:
```
build_x64\rundir\RelWithDebInfo\Media-Studio-Installer.exe
```

**Installer Features:**
- ✅ 295 MB all-in-one executable
- ✅ Professional Modern UI installer
- ✅ License agreement page
- ✅ Selectable components (Core, Start Menu, Desktop shortcuts)
- ✅ Admin elevation
- ✅ Windows registry integration
- ✅ Full uninstaller support
- ✅ x64 architecture verification

## Quick Start

### Prerequisites

1. **Build the project** (if not already done):
   ```bash
   cmake --preset=windows-x64-release
   cmake --build --preset=windows-x64-release
   ```

2. **NSIS** (already installed for you)
   - The build system will use: `C:\Program Files (x86)\NSIS\makensis.exe`

### Build the Installer

**Option 1: Using the batch file (Simplest)**

```cmd
cd installer
build-installer.bat
```

**Option 2: With custom version**

```cmd
build-installer.bat 1.2.3
```

**Option 3: With custom paths**

```cmd
build-installer.bat 1.2.3 "C:\custom\output\path"
```

**Option 4: Manual NSIS compilation**

```cmd
"C:\Program Files (x86)\NSIS\makensis.exe" media-studio.nsi
```

## File Structure

```
installer/
├── media-studio.nsi          # Main NSIS installer script (NSIS script language)
├── build-installer.bat      # Batch build automation (Windows batch)
└── README.md               # This documentation
```

## How to Use the Installer

1. Run `Media-Studio-Installer.exe`
2. Accept the GPLv2 license
3. Select installation components:
   - **Media Studio Core** (required) - Main application
   - **Start Menu Shortcuts** (optional)
   - **Desktop Shortcut** (optional)
4. Choose installation directory (default: `C:\Program Files\Media Studio`)
5. Complete the installation
6. Find shortcuts in Start Menu or Desktop

## Customization Guide

### Change Application Name

Edit `media-studio.nsi`, line 11:
```nsi
Name "Your App Name"
```

Also update:
- Line 68: Install directory registry keys
- Line 71-72: Shortcut labels
- Version strings (lines 35-39)

### Change Installation Directory

Line 12 in `media-studio.nsi`:
```nsi
InstallDir "$PROGRAMFILES64\Your App Name"
```

### Change Version Number

Lines 35-39 in `media-studio.nsi`:
```nsi
VIProductVersion "1.2.3.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "1.2.3"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "1.2.3"
```

### Change Build Output Path

The script looks for binaries in: `..\build_x64\rundir\RelWithDebInfo`

Preserve the runtime directory layout when changing the source paths:
```nsi
SetOutPath "$INSTDIR\bin\64bit"
File /r "C:\path\to\your\build\bin\64bit\*.*"
SetOutPath "$INSTDIR\data"
File /r "C:\path\to\your\build\data\*.*"
SetOutPath "$INSTDIR\obs-plugins"
File /r "C:\path\to\your\build\obs-plugins\*.*"
```

Shortcuts target `bin\64bit\obs64.exe` and use `bin\64bit` as their
working directory so the application can locate `..\..\data` and its plugins.

### Add More Languages

Add to `media-studio.nsi` (around line 29):
```nsi
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Russian"
```

### Reduce Installer Size

The installer includes all dependencies (300MB). To reduce:

1. **Remove debug symbols**: Delete `.pdb` files from build output
   ```powershell
   Get-ChildItem *.pdb -Recurse | Remove-Item
   ```

2. **Remove unused plugins**: Delete unused plugin folders
   ```
   build_x64\rundir\RelWithDebInfo\obs-plugins\
   ```

3. **Enable UPX compression** (advanced):
   Edit `media-studio.nsi`, add after line 32:
   ```nsi
   SetCompress off  ; then manually use UPX on output
   ```

### Add Custom Files

To include additional files, add more `File` commands in the "Media Studio Core" section:
```nsi
File /r "C:\path\to\extra\files\*.*"
```

### Exclude Specific Files

Modify the `File` commands with exclusion patterns:
```nsi
File /r /x "*.pdb" "..\build_x64\rundir\RelWithDebInfo\bin\64bit\*.*"
```

## Troubleshooting

### "NSIS not found"
Install NSIS:
```cmd
winget install -e --id NSIS.NSIS
```

### "Build path not found"
Make sure you've built the project:
```bash
cmake --preset=windows-x64-release
cmake --build --preset=windows-x64-release
```

### Installer won't run
- Ensure you're on Windows x64 (installer checks for this)
- Run as Administrator if prompted
- Temporarily disable antivirus (some AV blocks new .exe files)

### Installer is too large
The 300MB size includes:
- OBS core library (libobs.dll, ~1.5MB)
- All video/audio codecs (FFmpeg, ~40MB)
- All plugins (~50MB)
- Qt framework (~30MB)
- Runtime dependencies

This is normal for a full streaming application. To reduce:
1. Delete `.pdb` debug files (saves ~100MB)
2. Remove unused plugins
3. Use zlib compression (already enabled, saves ~30%)

### "Failed to find locale/en-US.ini"

This means the runtime data is missing or installed in the wrong location.
The installation must include:

```
bin\64bit\obs64.exe
data\obs-studio\locale\en-US.ini
obs-plugins\64bit\
```

Rebuild with the corrected `media-studio.nsi` and reinstall. Do not launch an
old `obs64.exe` left in the installation root by an earlier installer; use
the updated shortcut or `bin\64bit\obs64.exe`.

### Shortcut not working after install
Verify the executable path in `media-studio.nsi` matches:
```
$INSTDIR\bin\64bit\obs64.exe
```

If your binary is elsewhere, update the shortcut paths (lines 67-68, 72-73).

### Uninstaller fails
- Run as Administrator
- Ensure Media Studio is not running
- Check File Permissions in Properties > Security

## Distribution

Your installer is ready to:

1. **Distribute via email or download link**
   ```
   Media-Studio-Installer.exe (294 MB)
   ```

2. **Deploy via Group Policy** (enterprise)
   - Copy .exe to network share
   - Create GPO to push installation
   - Use `/S` for silent installation:
     ```cmd
     Media-Studio-Installer.exe /S /D=C:\Program Files\Media Studio
     ```

3. **Host on website/release page**
   - Upload to GitHub Releases, SourceForge, etc.
   - Include checksums (SHA256)
   - Include download link on main website

4. **Sign the installer** (optional but recommended)
   - Get a code-signing certificate
   - Sign with SignTool:
     ```cmd
     signtool sign /f cert.pfx /p password /t http://timestamp.server /d "Media Studio" Media-Studio-Installer.exe
     ```

### Silent Installation

Users can install silently with:
```cmd
Media-Studio-Installer.exe /S /D=C:\Program Files\Media Studio
```

### Command Line Options

```
/S              - Silent mode
/D=path         - Installation directory
/?              - Help
```

## NSIS Documentation

For advanced customization:
- **NSIS Manual**: https://nsis.sourceforge.io/Docs/
- **Modern UI Guide**: https://nsis.sourceforge.io/Docs/Modern%20UI/Readme.html
- **Script Examples**: https://nsis.sourceforge.io/Examples
- **Plugin Reference**: https://nsis.sourceforge.io/Plugins

## License

This installer script is provided under the same license as Media Studio (GPLv2).
See `frontend\data\license\gplv2.txt` for details.

## Next Steps

1. ✅ **Test the installer**
   - Double-click `Media-Studio-Installer.exe`
   - Go through installation wizard
   - Verify shortcuts work
   - Test uninstall

2. 📦 **Distribute**
   - Upload to your distribution platform
   - Share download link with team
   - Create release notes

3. 🔐 **Optional: Sign the installer**
   - Purchase code-signing certificate
   - Use SignTool to sign (see Distribution section)

4. 📊 **Track usage** (optional)
   - Add analytics to track downloads
   - Monitor user feedback

---

**Questions?** Refer to the NSIS documentation or modify `media-studio.nsi` to customize your installer.
