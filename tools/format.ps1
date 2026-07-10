[CmdletBinding()]
param(
    [string[]] $Paths = @('src', 'reference'),

    [switch] $Write,

    [switch] $List,

    [switch] $IncludeVendor
)

. "$PSScriptRoot\common-build.ps1"

$repoRoot = Get-RepoRoot
Set-Location -LiteralPath $repoRoot

function Get-FormatCandidateFiles {
    param(
        [string[]] $SearchPaths,
        [switch] $IncludeVendorFiles
    )

    $existingPaths = @()
    foreach ($path in $SearchPaths) {
        $resolved = Resolve-RepoPath -Path $path
        if (Test-Path -LiteralPath $resolved) {
            $existingPaths += $resolved
        }
    }

    if ($existingPaths.Count -eq 0) {
        return @()
    }

    if (Get-Command rg -ErrorAction SilentlyContinue) {
        $rgArgs = @('--files')
        if (-not $IncludeVendorFiles) {
            $rgArgs += @('-g', '!vendor/**')
        }
        $rgArgs += @(
            '-g', '*.c',
            '-g', '*.cc',
            '-g', '*.cpp',
            '-g', '*.h',
            '-g', '*.hh',
            '-g', '*.hpp'
        )
        $rgArgs += $existingPaths

        return @(& rg @rgArgs | Sort-Object)
    }

    $extensions = @('.c', '.cc', '.cpp', '.h', '.hh', '.hpp')
    $files = foreach ($path in $existingPaths) {
        Get-ChildItem -LiteralPath $path -Recurse -File | Where-Object {
            $extensions -contains $_.Extension.ToLowerInvariant() -and
            ($IncludeVendorFiles -or $_.FullName -notlike (Join-Path $repoRoot 'vendor\*'))
        } | ForEach-Object { $_.FullName }
    }

    return @($files | Sort-Object)
}

$files = Get-FormatCandidateFiles -SearchPaths $Paths -IncludeVendorFiles:$IncludeVendor
if ($files.Count -eq 0) {
    Write-Host 'No C/C++ source files found.'
    exit 0
}

if ($List) {
    $files
    exit 0
}

if (-not (Get-Command clang-format -ErrorAction SilentlyContinue)) {
    throw 'clang-format was not found on PATH. Install LLVM/clang-format, or run with -List to inspect candidate files.'
}

if ($Write) {
    Write-Host "Formatting $($files.Count) file(s)."
    Invoke-NativeCommand -FilePath 'clang-format' -Arguments (@('-i') + $files)
} else {
    Write-Host "Checking formatting for $($files.Count) file(s). Use -Write to apply changes."
    Invoke-NativeCommand -FilePath 'clang-format' -Arguments (@('--dry-run', '--Werror') + $files)
}
