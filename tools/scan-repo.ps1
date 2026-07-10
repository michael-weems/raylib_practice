[CmdletBinding()]
param(
    [switch] $IncludeVendor,

    [switch] $IncludeOut,

    [switch] $NoFiles,

    [switch] $NoTodos
)

. "$PSScriptRoot\common-build.ps1"

$repoRoot = Get-RepoRoot
Set-Location -LiteralPath $repoRoot

$rgGlobs = @('-g', '!.git/**')
if (-not $IncludeVendor) {
    $rgGlobs += @('-g', '!vendor/**')
}
if (-not $IncludeOut) {
    $rgGlobs += @('-g', '!out/**')
}

Write-Host "Repo: $repoRoot"

if (Get-Command git -ErrorAction SilentlyContinue) {
    Write-Host ''
    Write-Host '== Git status =='
    & git status --short
    if ($LASTEXITCODE -ne 0) {
        Write-Host '(git status failed)'
    }
}

Write-Host ''
Write-Host '== Key entrypoints =='
$entrypoints = @(
    'CONTEXT.md',
    'CURRENT_STEP.md',
    'CMakeLists.txt',
    'src\main.cpp',
    'reference\CMakeLists.txt',
    'reference\main.cpp',
    'reference\tests_core.cpp',
    'tools\build-src.ps1',
    'tools\build-reference.ps1',
    'tools\check-builds.ps1'
)
foreach ($entrypoint in $entrypoints) {
    if (Test-Path -LiteralPath $entrypoint) {
        Write-Host $entrypoint
    }
}

if (-not $NoFiles) {
    Write-Host ''
    Write-Host '== Files =='
    if (Get-Command rg -ErrorAction SilentlyContinue) {
        & rg --files @rgGlobs | Sort-Object
    } else {
        Get-ChildItem -Recurse -File | ForEach-Object { $_.FullName.Substring($repoRoot.Length + 1) } | Sort-Object
    }
}

if (-not $NoTodos) {
    Write-Host ''
    Write-Host '== Task markers =='
    if (Get-Command rg -ErrorAction SilentlyContinue) {
        $markerWords = @('TO' + 'DO', 'FIX' + 'ME', 'HA' + 'CK', 'BU' + 'G', 'X' + 'XX')
        $markerPattern = '\b(' + ($markerWords -join '|') + ')\b'
        $matches = & rg -n --hidden @rgGlobs $markerPattern . 2>$null
        if ($LASTEXITCODE -eq 0) {
            $matches
        } elseif ($LASTEXITCODE -eq 1) {
            Write-Host '(none)'
        } else {
            Write-Host '(marker scan failed)'
        }
    } else {
        Write-Host '(ripgrep not found)'
    }
}
