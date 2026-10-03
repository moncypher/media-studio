# Build NSIS Installer for Media Studio
# Usage: .\build-installer.ps1 -BinaryPath C:\path\to\build -OutputPath C:\output

param(
    [string]$BinaryPath = "$PSScriptRoot\..\build_x64\rundir\RelWithDebInfo",
    [string]$OutputPath = "$PSScriptRoot\..\build_x64\rundir\RelWithDebInfo",
    [string]$Version = "30.0.0",
    [string]$InstallerScript = "$PSScriptRoot\media-studio.nsi"
)

$ErrorActionPreference = "Continue"

Write-Host "Media Studio Installer Builder" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

# Check NSIS
$nsisPath = "C:\Program Files (x86)\NSIS\makensis.exe"
if (-not (Test-Path $nsisPath)) {
    Write-Host "ERROR: NSIS not found at $nsisPath" -ForegroundColor Red
    exit 1
}
Write-Host "✓ NSIS found" -ForegroundColor Green

# Check build output
if (-not (Test-Path $BinaryPath)) {
    Write-Host "ERROR: Build path not found: $BinaryPath" -ForegroundColor Red
    exit 1
}
Write-Host "✓ Build output found" -ForegroundColor Green

# Check installer script
if (-not (Test-Path $InstallerScript)) {
    Write-Host "ERROR: Installer script not found: $InstallerScript" -ForegroundColor Red
    exit 1
}
Write-Host "✓ Installer script found" -ForegroundColor Green

# Create output directory
if (-not (Test-Path $OutputPath)) {
    New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
}

Write-Host ""
Write-Host "Building installer..." -ForegroundColor Cyan
Write-Host "  Version: $Version" -ForegroundColor Gray
Write-Host "  Output: $OutputPath" -ForegroundColor Gray
Write-Host ""

# Build
& $nsisPath "/DVERSION=$Version" "/DOUTPUT_PATH=$OutputPath" "/V4" "$InstallerScript"

if ($LASTEXITCODE -eq 0) {
    $installerFile = "$OutputPath\Media-Studio-Installer.exe"
    if (Test-Path $installerFile) {
        $fileSize = [Math]::Round((Get-Item $installerFile).Length / 1MB, 2)
        Write-Host ""
        Write-Host "✓ SUCCESS! Installer created:" -ForegroundColor Green
        Write-Host "  $installerFile" -ForegroundColor Cyan
        Write-Host "  Size: $fileSize MB" -ForegroundColor Gray
    }
} else {
    Write-Host "ERROR: NSIS failed with exit code $LASTEXITCODE" -ForegroundColor Red
    exit 1
}
