<#
.SYNOPSIS
    Build script for polygon simplification project
.DESCRIPTION
    Compiles the APSC algorithm implementation using Visual Studio's cl.exe
.EXAMPLE
    .\build.ps1           # Build in Release mode
    .\build.ps1 -Debug    # Build in Debug mode
    .\build.ps1 -Clean    # Clean build artifacts
#>

param(
    [switch]$Debug,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

# Find Visual Studio installation
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vsWhere)) {
    Write-Error "Visual Studio not found. Please install Visual Studio with C++ tools."
    exit 1
}

$vsPath = & $vsWhere -latest -property installationPath
$vcVarsAll = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"

if (-not (Test-Path $vcVarsAll)) {
    Write-Error "vcvars64.bat not found at $vcVarsAll"
    exit 1
}

# Output directory
$outDir = "bin"
$target = "simplify.exe"

if ($Clean) {
    Write-Host "Cleaning build artifacts..." -ForegroundColor Yellow
    if (Test-Path $outDir) { Remove-Item -Recurse -Force $outDir }
    if (Test-Path $target) { Remove-Item -Force $target }
    Write-Host "Clean complete." -ForegroundColor Green
    exit 0
}

# Create output directory
if (-not (Test-Path $outDir)) {
    New-Item -ItemType Directory -Path $outDir | Out-Null
}

# Compile options
$cxxFlags = "/std:c++17 /EHsc /W4 /nologo"
if ($Debug) {
    $cxxFlags += " /Od /Zi /MDd"
    Write-Host "Building in DEBUG mode..." -ForegroundColor Yellow
} else {
    $cxxFlags += " /O2 /MD"
    Write-Host "Building in RELEASE mode..." -ForegroundColor Green
}

# Source files
$sources = "src\main.cpp"

# Build command
$buildCmd = @"
call "$vcVarsAll" >nul 2>&1
cl.exe $cxxFlags /Fe:$target $sources
"@

# Execute build
Write-Host "Compiling..." -ForegroundColor Cyan
cmd /c $buildCmd

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nBuild successful: $target" -ForegroundColor Green
    Write-Host "`nUsage:" -ForegroundColor Cyan
    Write-Host "  .\simplify.exe <input.csv> <target_vertices>"
    Write-Host "`nExample:"
    Write-Host "  .\simplify.exe ..\test_cases\input_rectangle_with_two_holes.csv 7"
} else {
    Write-Host "`nBuild failed!" -ForegroundColor Red
    exit 1
}
