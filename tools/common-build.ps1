Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$script:VcVars64Imported = $false

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

    foreach ($line in $lines) {
        $separator = $line.IndexOf('=')
        if ($separator -le 0) {
            continue
        }

        $name = $line.Substring(0, $separator)
        $value = $line.Substring($separator + 1)
        Set-Item -Path "Env:$name" -Value $value
    }

    $script:VcVars64Imported = $true
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

        [string] $Target = '',

        [switch] $CleanFirst
    )

    $sourcePath = Resolve-RepoPath -Path $SourceDir
    $buildPath = Resolve-RepoPath -Path $BuildDir

    Import-VcVars64Environment

    Invoke-NativeCommand -FilePath 'cmake' -Arguments @(
        '-S', $sourcePath,
        '-B', $buildPath,
        '-G', $Generator,
        "-DCMAKE_BUILD_TYPE=$BuildType"
    ) | Out-Host

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
