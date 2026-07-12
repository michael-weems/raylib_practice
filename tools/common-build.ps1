Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$script:VcVars64Imported = $false

function Get-RequestedCompiler {
    param(
        [string] $Compiler = $env:SW_RENDER_COMPILER
    )

    if ([string]::IsNullOrWhiteSpace($Compiler)) {
        return 'ClangCl'
    }

    if ($Compiler -notin @('ClangCl', 'MSVC')) {
        throw "Unknown compiler '$Compiler'. Expected ClangCl or MSVC."
    }

    return $Compiler
}

function Get-RepoRoot {
    return (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
}

function Resolve-RepoPath {
    param(
        [Parameter(Mandatory = $true)]
        [string] $Path,

        [string] $BasePath = (Get-RepoRoot)
    )

    if ([System.IO.Path]::IsPathRooted($Path)) {
        return [System.IO.Path]::GetFullPath($Path)
    }

    return [System.IO.Path]::GetFullPath((Join-Path $BasePath $Path))
}

function Find-VcVars64 {
    param(
        [string] $RequestedPath = $env:VCVARS64
    )

    $candidates = @()

    if (-not [string]::IsNullOrWhiteSpace($RequestedPath)) {
        $candidates += $RequestedPath
    }

    $programFilesX86 = ${env:ProgramFiles(x86)}
    if (-not [string]::IsNullOrWhiteSpace($programFilesX86)) {
        $vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
        if (Test-Path -LiteralPath $vswhere) {
            $installPath = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null | Select-Object -First 1
            if ($LASTEXITCODE -eq 0 -and -not [string]::IsNullOrWhiteSpace($installPath)) {
                $candidates += (Join-Path $installPath 'VC\Auxiliary\Build\vcvars64.bat')
            }
        }
    }

    $programFiles = $env:ProgramFiles
    if (-not [string]::IsNullOrWhiteSpace($programFiles)) {
        $candidates += @(
            (Join-Path $programFiles 'Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat'),
            (Join-Path $programFiles 'Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat'),
            (Join-Path $programFiles 'Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat'),
            (Join-Path $programFiles 'Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat'),
            (Join-Path $programFiles 'Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat'),
            (Join-Path $programFiles 'Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat')
        )
    }

    foreach ($candidate in $candidates) {
        if (-not [string]::IsNullOrWhiteSpace($candidate) -and (Test-Path -LiteralPath $candidate)) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    throw "Could not find vcvars64.bat. Install Visual Studio C++ build tools or set VCVARS64 to the full path."
}

function Find-ClangCl {
    param(
        [string] $RequestedPath = $env:CLANG_CL
    )

    $candidates = @()
    if (-not [string]::IsNullOrWhiteSpace($RequestedPath)) {
        $candidates += $RequestedPath
    }

    if (-not [string]::IsNullOrWhiteSpace($env:ProgramFiles)) {
        $candidates += (Join-Path $env:ProgramFiles 'LLVM\bin\clang-cl.exe')
    }

    $command = Get-Command clang-cl.exe -ErrorAction SilentlyContinue
    if ($command) {
        $candidates += $command.Source
    }

    if (-not [string]::IsNullOrWhiteSpace($env:ProgramFiles)) {
        $candidates += (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-cl.exe')
        $candidates += (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin\clang-cl.exe')
    }

    foreach ($candidate in $candidates) {
        if (-not [string]::IsNullOrWhiteSpace($candidate) -and (Test-Path -LiteralPath $candidate)) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    throw 'Could not find clang-cl.exe. Install LLVM, add it to PATH, or set CLANG_CL to its full path.'
}

function Find-ClangFormat {
    param(
        [string] $RequestedPath = $env:CLANG_FORMAT
    )

    $candidates = @()
    if (-not [string]::IsNullOrWhiteSpace($RequestedPath)) {
        $candidates += $RequestedPath
    }

    if (-not [string]::IsNullOrWhiteSpace($env:ProgramFiles)) {
        $candidates += (Join-Path $env:ProgramFiles 'LLVM\bin\clang-format.exe')
    }

    $command = Get-Command clang-format.exe -ErrorAction SilentlyContinue
    if ($command) {
        $candidates += $command.Source
    }

    if (-not [string]::IsNullOrWhiteSpace($env:ProgramFiles)) {
        $candidates += (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-format.exe')
        $candidates += (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin\clang-format.exe')
    }

    foreach ($candidate in $candidates) {
        if (-not [string]::IsNullOrWhiteSpace($candidate) -and (Test-Path -LiteralPath $candidate)) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    throw 'Could not find clang-format.exe. Install LLVM, add it to PATH, or set CLANG_FORMAT to its full path.'
}

function Import-VcVars64Environment {
    if ($script:VcVars64Imported) {
        return
    }

    $vcvars64 = Find-VcVars64
    Write-Host "Using MSVC environment: $vcvars64"

    $cmdLine = "call `"$vcvars64`" >nul && set"
    $lines = & cmd.exe /d /s /c $cmdLine
    if ($LASTEXITCODE -ne 0) {
        throw "vcvars64.bat failed with exit code $LASTEXITCODE"
    }

    $visualStudioPath = $lines | Where-Object {
        $_.StartsWith('PATH=', [System.StringComparison]::OrdinalIgnoreCase)
    } | Select-Object -First 1

    foreach ($line in $lines) {
        $separator = $line.IndexOf('=')
        if ($separator -le 0) {
            continue
        }

        $name = $line.Substring(0, $separator)
        if ($name -notmatch '^[A-Za-z_][A-Za-z0-9_.()]*$') {
            continue
        }
        if ($name.Equals('Path', [System.StringComparison]::OrdinalIgnoreCase)) {
            continue
        }

        $value = $line.Substring($separator + 1)
        Set-Item -LiteralPath "Env:$name" -Value $value
    }

    if (-not [string]::IsNullOrWhiteSpace($visualStudioPath)) {
        $separator = $visualStudioPath.IndexOf('=')
        $env:Path = $visualStudioPath.Substring($separator + 1)
    }

    $script:VcVars64Imported = $true
}

function Get-CompilerPath {
    param(
        [Parameter(Mandatory = $true)]
        [ValidateSet('ClangCl', 'MSVC')]
        [string] $Compiler
    )

    if ($Compiler -eq 'ClangCl') {
        return Find-ClangCl
    }

    $command = Get-Command cl.exe -ErrorAction SilentlyContinue
    if (-not $command) {
        throw 'cl.exe was not found after importing the Visual Studio build environment.'
    }

    return $command.Source
}

function Get-CachedCompilerPath {
    param(
        [Parameter(Mandatory = $true)]
        [string] $BuildPath
    )

    $cachePath = Join-Path $BuildPath 'CMakeCache.txt'
    if (-not (Test-Path -LiteralPath $cachePath)) {
        return ''
    }

    $match = Select-String -LiteralPath $cachePath -Pattern '^CMAKE_C_COMPILER:[^=]+=(.+)$' | Select-Object -First 1
    if (-not $match) {
        return ''
    }

    return $match.Matches[0].Groups[1].Value
}

function Get-CachedGenerator {
    param(
        [Parameter(Mandatory = $true)]
        [string] $BuildPath
    )

    $cachePath = Join-Path $BuildPath 'CMakeCache.txt'
    if (-not (Test-Path -LiteralPath $cachePath)) {
        return ''
    }

    $match = Select-String -LiteralPath $cachePath -Pattern '^CMAKE_GENERATOR:INTERNAL=(.+)$' | Select-Object -First 1
    if (-not $match) {
        return ''
    }

    return $match.Matches[0].Groups[1].Value
}

function Test-CMakeCacheNeedsRefresh {
    param(
        [Parameter(Mandatory = $true)]
        [string] $BuildPath,

        [Parameter(Mandatory = $true)]
        [string] $CompilerPath,

        [Parameter(Mandatory = $true)]
        [string] $Generator
    )

    $cachedGenerator = Get-CachedGenerator -BuildPath $BuildPath
    if (-not [string]::IsNullOrWhiteSpace($cachedGenerator) -and $cachedGenerator -ne $Generator) {
        return $true
    }

    $cachedCompiler = Get-CachedCompilerPath -BuildPath $BuildPath
    if ([string]::IsNullOrWhiteSpace($cachedCompiler)) {
        return $false
    }
    if (-not (Test-Path -LiteralPath $cachedCompiler)) {
        return $true
    }

    $cachedFullPath = [System.IO.Path]::GetFullPath($cachedCompiler)
    $requestedFullPath = [System.IO.Path]::GetFullPath($CompilerPath)
    return -not $cachedFullPath.Equals($requestedFullPath, [System.StringComparison]::OrdinalIgnoreCase)
}

function Format-NativeCommand {
    param(
        [Parameter(Mandatory = $true)]
        [string] $FilePath,

        [string[]] $Arguments = @()
    )

    $parts = @($FilePath) + $Arguments
    return ($parts | ForEach-Object {
        if ($_ -match '\s') {
            return '"' + $_ + '"'
        }

        return $_
    }) -join ' '
}

function Invoke-NativeCommand {
    param(
        [Parameter(Mandatory = $true)]
        [string] $FilePath,

        [string[]] $Arguments = @()
    )

    Write-Host ('> ' + (Format-NativeCommand -FilePath $FilePath -Arguments $Arguments))
    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$FilePath failed with exit code $LASTEXITCODE"
    }
}

function Invoke-CMakeBuild {
    param(
        [Parameter(Mandatory = $true)]
        [string] $SourceDir,

        [Parameter(Mandatory = $true)]
        [string] $BuildDir,

        [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
        [string] $BuildType = 'Debug',

        [string] $Generator = $(if ($env:CMAKE_GENERATOR) { $env:CMAKE_GENERATOR } else { 'Ninja' }),

        [ValidateSet('ClangCl', 'MSVC')]
        [string] $Compiler = (Get-RequestedCompiler),

        [string] $Target = '',

        [switch] $CleanFirst
    )

    $sourcePath = Resolve-RepoPath -Path $SourceDir
    $buildPath = Resolve-RepoPath -Path $BuildDir

    Import-VcVars64Environment

    $compilerPath = Get-CompilerPath -Compiler $Compiler
    Write-Host "Using C/C++ compiler: $compilerPath"

    $configureArgs = @()
    if (Test-CMakeCacheNeedsRefresh -BuildPath $buildPath -CompilerPath $compilerPath -Generator $Generator) {
        Write-Host 'Compiler changed or cached compiler disappeared; refreshing the CMake cache.'
        $configureArgs += '--fresh'
    }

    $configureArgs += @(
        '-S', $sourcePath,
        '-B', $buildPath,
        '-G', $Generator,
        "-DCMAKE_BUILD_TYPE=$BuildType",
        "-DCMAKE_C_COMPILER:FILEPATH=$compilerPath",
        "-DCMAKE_CXX_COMPILER:FILEPATH=$compilerPath"
    )
    Invoke-NativeCommand -FilePath 'cmake' -Arguments $configureArgs | Out-Host

    $buildArgs = @('--build', $buildPath, '--config', $BuildType)
    if (-not [string]::IsNullOrWhiteSpace($Target)) {
        $buildArgs += @('--target', $Target)
    }
    if ($CleanFirst) {
        $buildArgs += '--clean-first'
    }

    Invoke-NativeCommand -FilePath 'cmake' -Arguments $buildArgs | Out-Host
    return $buildPath
}

function Get-BuiltExecutablePath {
    param(
        [Parameter(Mandatory = $true)]
        [string] $BuildDir,

        [Parameter(Mandatory = $true)]
        [string] $TargetName,

        [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
        [string] $BuildType = 'Debug'
    )

    $buildPath = Resolve-RepoPath -Path $BuildDir
    $candidates = @(
        (Join-Path $buildPath "$TargetName.exe"),
        (Join-Path (Join-Path $buildPath $BuildType) "$TargetName.exe")
    )

    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    return $candidates[0]
}
