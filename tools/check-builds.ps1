[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string] $BuildType = 'Debug',

    [string] $Generator = $(if ($env:CMAKE_GENERATOR) { $env:CMAKE_GENERATOR } else { 'Ninja' }),

    [ValidateSet('ClangCl', 'MSVC')]
    [string] $Compiler = $(if ($env:SW_RENDER_COMPILER) { $env:SW_RENDER_COMPILER } else { 'ClangCl' }),

    [switch] $SkipTests,

    [switch] $CleanFirst
)

. "$PSScriptRoot\common-build.ps1"

$repoRoot = Get-RepoRoot

Write-Host 'Checking src build...'
$srcBuildPath = Invoke-CMakeBuild `
    -SourceDir $repoRoot `
    -BuildDir 'out\build' `
    -BuildType $BuildType `
    -Generator $Generator `
    -Compiler $Compiler `
    -Target 'software_renderer' `
    -CleanFirst:$CleanFirst

Write-Host 'Checking reference build...'
$referenceBuildPath = Invoke-CMakeBuild `
    -SourceDir (Join-Path $repoRoot 'reference') `
    -BuildDir 'out\reference-build' `
    -BuildType $BuildType `
    -Generator $Generator `
    -Compiler $Compiler `
    -CleanFirst:$CleanFirst

if (-not $SkipTests) {
    Write-Host 'Running registered reference tests...'
    Invoke-NativeCommand -FilePath 'ctest' -Arguments @(
        '--test-dir', $referenceBuildPath,
        '--output-on-failure',
        '-C', $BuildType
    )
}

Write-Host 'Build check completed.'
