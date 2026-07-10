[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string] $BuildType = 'Debug',

    [string] $Generator = $(if ($env:CMAKE_GENERATOR) { $env:CMAKE_GENERATOR } else { 'Ninja' }),

    [string] $BuildDir = 'out\build',

    [switch] $Run,

    [switch] $Debugger,

    [switch] $CleanFirst
)

. "$PSScriptRoot\common-build.ps1"

$repoRoot = Get-RepoRoot
$buildPath = Invoke-CMakeBuild `
    -SourceDir $repoRoot `
    -BuildDir $BuildDir `
    -BuildType $BuildType `
    -Generator $Generator `
    -Target 'software_renderer' `
    -CleanFirst:$CleanFirst

$exePath = Get-BuiltExecutablePath -BuildDir $buildPath -TargetName 'software_renderer' -BuildType $BuildType
if (-not (Test-Path -LiteralPath $exePath)) {
    throw "Build completed, but expected executable was not found: $exePath"
}

Write-Host "Built src application: $exePath"

if ($Run -or $Debugger) {
    if ($Debugger) {
        Invoke-NativeCommand -FilePath 'raddbg' -Arguments @($exePath)
    } else {
        Invoke-NativeCommand -FilePath $exePath
    }
}
