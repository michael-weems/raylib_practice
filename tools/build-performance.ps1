[CmdletBinding()]
param(
    [ValidateSet('src', 'reference')]
    [string] $App = 'reference',

    [ValidateSet('Release', 'RelWithDebInfo')]
    [string] $BuildType = 'RelWithDebInfo',

    [string] $Generator = $(if ($env:CMAKE_GENERATOR) { $env:CMAKE_GENERATOR } else { 'Ninja' }),

    [ValidateSet('ClangCl', 'MSVC')]
    [string] $Compiler = $(if ($env:SW_RENDER_COMPILER) { $env:SW_RENDER_COMPILER } else { 'ClangCl' }),

    [switch] $Run,

    [switch] $CleanFirst
)

. "$PSScriptRoot\common-build.ps1"

$repoRoot = Get-RepoRoot
if ($App -eq 'src') {
    $sourceDir = $repoRoot
    $buildDir = 'out\performance-src'
    $targetName = 'software_renderer'
} else {
    $sourceDir = Join-Path $repoRoot 'reference'
    $buildDir = 'out\performance-reference'
    $targetName = 'reference_software_renderer'
}

$buildPath = Invoke-CMakeBuild `
    -SourceDir $sourceDir `
    -BuildDir $buildDir `
    -BuildType $BuildType `
    -Generator $Generator `
    -Compiler $Compiler `
    -Target $targetName `
    -CleanFirst:$CleanFirst

$executablePath = Get-BuiltExecutablePath `
    -BuildDir $buildPath `
    -TargetName $targetName `
    -BuildType $BuildType

if (-not (Test-Path -LiteralPath $executablePath)) {
    throw "Expected performance executable was not found: $executablePath"
}

Write-Host "Built optimized $App application: $executablePath"
if ($Run) {
    $workingDirectory = Split-Path -Parent $executablePath
    Push-Location -LiteralPath $workingDirectory
    try {
        Invoke-NativeCommand -FilePath $executablePath
    } finally {
        Pop-Location
    }
}
