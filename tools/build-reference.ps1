[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string] $BuildType = 'RelWithDebInfo',

    [string] $Generator = $(if ($env:CMAKE_GENERATOR) { $env:CMAKE_GENERATOR } else { 'Ninja' }),

    [string] $BuildDir = 'out\reference-build',

    [switch] $Run,

    [switch] $RunTests,

    [switch] $Debugger,

    [switch] $CleanFirst
)

. "$PSScriptRoot\common-build.ps1"

$repoRoot = Get-RepoRoot
$referenceRoot = Join-Path $repoRoot 'reference'

$target = 'reference_software_renderer'
if ($RunTests) {
    $target = ''
}

$buildPath = Invoke-CMakeBuild `
    -SourceDir $referenceRoot `
    -BuildDir $BuildDir `
    -BuildType $BuildType `
    -Generator $Generator `
    -Target $target `
    -CleanFirst:$CleanFirst

$exePath = Get-BuiltExecutablePath -BuildDir $buildPath -TargetName 'reference_software_renderer' -BuildType $BuildType
if (-not (Test-Path -LiteralPath $exePath)) {
    throw "Build completed, but expected executable was not found: $exePath"
}

Write-Host "Built reference application: $exePath"

if ($RunTests) {
    Invoke-NativeCommand -FilePath 'ctest' -Arguments @(
        '--test-dir', $buildPath,
        '--output-on-failure',
        '-C', $BuildType
    )
}

if ($Run -or $Debugger) {
    if ($Debugger) {
        Invoke-NativeCommand -FilePath 'raddbg' -Arguments @($exePath)
    } else {
        Invoke-NativeCommand -FilePath $exePath
    }
}
