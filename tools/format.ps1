[CmdletBinding()]
param(
    [string[]] $Paths = @('src', 'reference'),

    [switch] $Write,

    [switch] $List
)

. "$PSScriptRoot\common-build.ps1"

$repoRoot = Get-RepoRoot
Set-Location -LiteralPath $repoRoot

function Get-FormatCandidateFiles {
    param(
        [string[]] $SearchPaths
    )

    $vendorRoot = (Resolve-RepoPath -Path 'vendor').TrimEnd('\', '/')
    $extensions = @('.c', '.cc', '.cpp', '.h', '.hh', '.hpp')
    $directoryPaths = @()
    $candidateFiles = @()
    foreach ($path in $SearchPaths) {
        $resolved = Resolve-RepoPath -Path $path
        if (-not (Test-Path -LiteralPath $resolved)) {
            continue
        }

        $pathIsVendor = $resolved.Equals($vendorRoot, [System.StringComparison]::OrdinalIgnoreCase)
        $pathIsInsideVendor = $resolved.StartsWith(
            $vendorRoot + [System.IO.Path]::DirectorySeparatorChar,
            [System.StringComparison]::OrdinalIgnoreCase
        )
        if ($pathIsVendor -or $pathIsInsideVendor) {
            throw "Formatting vendored code is prohibited: $resolved"
        }

        if (Test-Path -LiteralPath $resolved -PathType Leaf) {
            if ($extensions -contains [System.IO.Path]::GetExtension($resolved).ToLowerInvariant()) {
                $candidateFiles += $resolved
            }
        } else {
            $directoryPaths += $resolved
        }
    }

    if ($directoryPaths.Count -ne 0 -and (Get-Command rg -ErrorAction SilentlyContinue)) {
        $rgArgs = @('--files')
        $rgArgs += @('-g', '!vendor/**')
        $rgArgs += @(
            '-g', '*.c',
            '-g', '*.cc',
            '-g', '*.cpp',
            '-g', '*.h',
            '-g', '*.hh',
            '-g', '*.hpp'
        )
        $rgArgs += $directoryPaths
        $candidateFiles += @(& rg @rgArgs)
        if ($LASTEXITCODE -gt 1) {
            throw "ripgrep failed while discovering format candidates with exit code $LASTEXITCODE"
        }
    } elseif ($directoryPaths.Count -ne 0) {
        foreach ($directoryPath in $directoryPaths) {
            $candidateFiles += @(Get-ChildItem -LiteralPath $directoryPath -Recurse -File |
                Where-Object { $extensions -contains $_.Extension.ToLowerInvariant() } |
                ForEach-Object { $_.FullName })
        }
    }

    $projectFiles = foreach ($candidate in $candidateFiles) {
        $fullPath = [System.IO.Path]::GetFullPath($candidate)
        $insideVendor = $fullPath.Equals($vendorRoot, [System.StringComparison]::OrdinalIgnoreCase)
        $insideVendor = $insideVendor -or $fullPath.StartsWith(
            $vendorRoot + [System.IO.Path]::DirectorySeparatorChar,
            [System.StringComparison]::OrdinalIgnoreCase
        )
        if (-not $insideVendor) {
            $fullPath
        }
    }

    return @($projectFiles | Sort-Object -Unique)
}

$files = @(Get-FormatCandidateFiles -SearchPaths $Paths)
if ($files.Count -eq 0) {
    Write-Host 'No C/C++ source files found.'
    exit 0
}

if ($List) {
    $files
    exit 0
}

$clangFormat = Find-ClangFormat

if ($Write) {
    Write-Host "Formatting $($files.Count) file(s)."
    Invoke-NativeCommand -FilePath $clangFormat -Arguments (@('-i') + $files)
} else {
    Write-Host "Checking formatting for $($files.Count) file(s). Use -Write to apply changes."
    Invoke-NativeCommand -FilePath $clangFormat -Arguments (@('--dry-run', '--Werror') + $files)
}
