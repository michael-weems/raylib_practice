[CmdletBinding()]
param(
    [ValidateSet('src', 'reference')]
    [string] $App = 'src',

    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string] $BuildType = 'Debug',

    [string] $Generator = $(if ($env:CMAKE_GENERATOR) { $env:CMAKE_GENERATOR } else { 'Ninja' }),

    [ValidateSet('ClangCl', 'MSVC')]
    [string] $Compiler = $(if ($env:SW_RENDER_COMPILER) { $env:SW_RENDER_COMPILER } else { 'ClangCl' }),

    [int] $TimeoutSeconds = 10,

    [switch] $NoBuild,

    [switch] $NoLaunch,

    [switch] $KeepOpen
)

. "$PSScriptRoot\common-build.ps1"

$repoRoot = Get-RepoRoot

if ($App -eq 'src') {
    $sourceDir = $repoRoot
    $buildDir = 'out\build'
    $targetName = 'software_renderer'
} else {
    $sourceDir = Join-Path $repoRoot 'reference'
    $buildDir = 'out\reference-build'
    $targetName = 'reference_software_renderer'
}

if (-not $NoBuild) {
    Invoke-CMakeBuild `
        -SourceDir $sourceDir `
        -BuildDir $buildDir `
        -BuildType $BuildType `
        -Generator $Generator `
        -Compiler $Compiler `
        -Target $targetName | Out-Null
}

$exePath = Get-BuiltExecutablePath -BuildDir $buildDir -TargetName $targetName -BuildType $BuildType
if (-not (Test-Path -LiteralPath $exePath)) {
    throw "Expected executable was not found: $exePath"
}

Write-Host "Smoke-test executable: $exePath"

if ($NoLaunch) {
    Write-Host 'NoLaunch set; build/executable check only.'
    exit 0
}

$workingDir = Split-Path -Parent $exePath
Write-Host "Launching $App app."
$process = Start-Process -FilePath $exePath -WorkingDirectory $workingDir -PassThru

if ($KeepOpen -or $TimeoutSeconds -le 0) {
    Write-Host "Process $($process.Id) is running. Close the window when visual inspection is done."
    exit 0
}

if ($process.WaitForExit($TimeoutSeconds * 1000)) {
    if ($process.ExitCode -ne 0) {
        throw "$App smoke test exited early with code $($process.ExitCode)."
    }

    Write-Host "$App smoke test exited normally."
    exit 0
}

Write-Host "$App stayed open for $TimeoutSeconds second(s); stopping it after the visual smoke window."
Stop-Process -Id $process.Id
