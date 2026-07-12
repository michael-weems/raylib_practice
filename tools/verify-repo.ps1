[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string] $BuildType = 'RelWithDebInfo',

    [string] $Generator = $(if ($env:CMAKE_GENERATOR) { $env:CMAKE_GENERATOR } else { 'Ninja' }),

    [ValidateSet('ClangCl', 'MSVC')]
    [string] $Compiler = $(if ($env:SW_RENDER_COMPILER) { $env:SW_RENDER_COMPILER } else { 'ClangCl' }),

    [switch] $CleanFirst,

    [switch] $RunFormatCheck,

    [string[]] $FormatPaths = @('reference')
)

. "$PSScriptRoot\common-build.ps1"

$repoRoot = Get-RepoRoot
Set-Location -LiteralPath $repoRoot

Write-Host 'Checking unstaged whitespace errors.'
Invoke-NativeCommand -FilePath 'git' -Arguments @(
    '-c', 'core.excludesfile=',
    'diff', '--check'
)

Write-Host 'Checking staged whitespace errors.'
Invoke-NativeCommand -FilePath 'git' -Arguments @(
    '-c', 'core.excludesfile=',
    'diff', '--cached', '--check'
)

Write-Host 'Vendored Raylib is treated as immutable and excluded from project verification.'

& "$PSScriptRoot\check-builds.ps1" `
    -BuildType $BuildType `
    -Generator $Generator `
    -Compiler $Compiler `
    -CleanFirst:$CleanFirst
if ($LASTEXITCODE -ne 0) {
    throw "Build/test verification failed with exit code $LASTEXITCODE"
}

if ($RunFormatCheck) {
    & "$PSScriptRoot\format.ps1" -Paths $FormatPaths
    if ($LASTEXITCODE -ne 0) {
        throw "Formatting verification failed with exit code $LASTEXITCODE"
    }
}

Write-Host 'Repository verification completed.'
