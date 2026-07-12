[CmdletBinding()]
param(
    [switch] $Stat,

    [switch] $NamesOnly
)

. "$PSScriptRoot\common-build.ps1"

$repoRoot = Get-RepoRoot
Set-Location -LiteralPath $repoRoot

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    throw 'git was not found on PATH.'
}

function Invoke-GitLines {
    param([string[]] $Arguments)

    $lines = & git -c core.excludesfile= -c core.autocrlf=false @Arguments 2>$null
    if ($LASTEXITCODE -ne 0) {
        throw "git $($Arguments -join ' ') failed with exit code $LASTEXITCODE"
    }

    return $lines
}

function Write-SectionLines {
    param(
        [string] $Title,
        [string[]] $Lines
    )

    Write-Host ''
    Write-Host "== $Title =="
    $items = @($Lines)
    if ($items.Count -eq 0) {
        Write-Host '(none)'
        return
    }

    $items
}

$staged = @(Invoke-GitLines -Arguments @('diff', '--cached', '--name-only'))
$unstaged = @(Invoke-GitLines -Arguments @('diff', '--name-only'))
$untracked = @(Invoke-GitLines -Arguments @('ls-files', '--others', '--exclude-standard'))
$allChanged = @($staged + $unstaged + $untracked | Sort-Object -Unique)

if ($NamesOnly) {
    $allChanged
    exit 0
}

Write-Host "Repo: $repoRoot"
Write-SectionLines -Title 'Staged' -Lines $staged
Write-SectionLines -Title 'Unstaged' -Lines $unstaged
Write-SectionLines -Title 'Untracked' -Lines $untracked

if ($Stat) {
    Write-Host ''
    Write-Host '== Diff stat =='
    & git -c core.excludesfile= diff --stat
    if ($LASTEXITCODE -ne 0) {
        throw "git diff --stat failed with exit code $LASTEXITCODE"
    }

    & git -c core.excludesfile= diff --cached --stat
    if ($LASTEXITCODE -ne 0) {
        throw "git diff --cached --stat failed with exit code $LASTEXITCODE"
    }
}

Write-Host ''
Write-Host '== Suggested checks =='
if ($allChanged.Count -eq 0) {
    Write-Host '(none)'
    exit 0
}

$checks = New-Object System.Collections.Generic.List[string]
if ($allChanged | Where-Object { $_ -like 'src/*' -or $_ -eq 'CMakeLists.txt' }) {
    $checks.Add('powershell -NoProfile -ExecutionPolicy Bypass -File tools\build-src.ps1')
}
if ($allChanged | Where-Object { $_ -like 'reference/*' }) {
    $checks.Add('powershell -NoProfile -ExecutionPolicy Bypass -File tools\build-reference.ps1 -RunTests')
}
if ($allChanged | Where-Object { $_ -like 'tools/*' -or $_ -like '*CMakeLists.txt' }) {
    $checks.Add('powershell -NoProfile -ExecutionPolicy Bypass -File tools\check-builds.ps1')
}
if ($allChanged | Where-Object { $_ -eq 'REPORT.md' -or $_ -eq 'package.json' -or $_ -eq 'package-lock.json' -or $_ -eq 'tools/docs.mjs' }) {
    $checks.Add('npm run docs:build')
}
if ($allChanged | Where-Object {
    $_ -like '*.cpp' -or $_ -like '*.h' -or $_ -like '*.c' -or $_ -eq '.clang-format'
}) {
    try {
        $null = Find-ClangFormat
        $checks.Add('powershell -NoProfile -ExecutionPolicy Bypass -File tools\format.ps1')
    } catch {
        $checks.Add('clang-format not found; tools\format.ps1 -List can still show candidate files')
    }
}

if ($checks.Count -eq 0) {
    Write-Host '(no build-sensitive files detected)'
} else {
    $checks | Sort-Object -Unique
}
