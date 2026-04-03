<#
.SYNOPSIS
    Run tests for polygon simplification project
.DESCRIPTION
    Runs the simplify executable against test cases and displays results
.EXAMPLE
    .\test.ps1            # Run all tests
    .\test.ps1 -Quick     # Run only quick tests (simple cases)
    .\test.ps1 -Verbose   # Show full output instead of summary
#>

param(
    [switch]$Quick,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

# Check if executable exists
$exe = ".\simplify.exe"
if (-not (Test-Path $exe)) {
    Write-Host "Error: $exe not found. Run .\build.ps1 first." -ForegroundColor Red
    exit 1
}

$testCasesDir = "..\test_cases"
if (-not (Test-Path $testCasesDir)) {
    Write-Error "Test cases directory not found: $testCasesDir"
    exit 1
}

# Test case definitions: [name, input_file, target, expected_output]
$simpleCases = @(
    @("rectangle_with_two_holes", "input_rectangle_with_two_holes.csv", 7, "output_rectangle_with_two_holes.txt"),
    @("cushion_with_hexagonal_hole", "input_cushion_with_hexagonal_hole.csv", 13, "output_cushion_with_hexagonal_hole.txt"),
    @("blob_with_two_holes", "input_blob_with_two_holes.csv", 17, "output_blob_with_two_holes.txt"),
    @("wavy_with_three_holes", "input_wavy_with_three_holes.csv", 21, "output_wavy_with_three_holes.txt"),
    @("lake_with_two_islands", "input_lake_with_two_islands.csv", 17, "output_lake_with_two_islands.txt")
)

$lakeCases = @(
    @("original_01", "input_original_01.csv", 99),
    @("original_02", "input_original_02.csv", 99),
    @("original_03", "input_original_03.csv", 99),
    @("original_04", "input_original_04.csv", 99),
    @("original_05", "input_original_05.csv", 99)
)

function Run-Test {
    param($name, $inputFile, $target)
    
    $inputPath = Join-Path $testCasesDir $inputFile
    if (-not (Test-Path $inputPath)) {
        Write-Host "  SKIP: Input file not found: $inputFile" -ForegroundColor Yellow
        return
    }
    
    Write-Host "  Testing: $name (target: $target)..." -ForegroundColor White -NoNewline
    
    try {
        $output = & $exe $inputPath $target 2>&1
        $lines = $output -split "`n"
        
        # Get last 3 lines (area summary)
        $summary = $lines | Select-Object -Last 3
        
        # Parse areas
        $inputArea = $null
        $outputArea = $null
        $displacement = $null
        
        foreach ($line in $summary) {
            if ($line -match "Total signed area in input:\s*(.+)") {
                $inputArea = $matches[1].Trim()
            }
            if ($line -match "Total signed area in output:\s*(.+)") {
                $outputArea = $matches[1].Trim()
            }
            if ($line -match "Total areal displacement:\s*(.+)") {
                $displacement = $matches[1].Trim()
            }
        }
        
        # Count vertices in output
        $vertexCount = ($lines | Where-Object { $_ -match "^\d+,\d+," }).Count
        
        # Check area preservation
        if ($inputArea -eq $outputArea) {
            Write-Host " PASS" -ForegroundColor Green -NoNewline
            Write-Host " (vertices: $vertexCount, displacement: $displacement)"
        } else {
            Write-Host " FAIL" -ForegroundColor Red
            Write-Host "    Input area:  $inputArea" -ForegroundColor Red
            Write-Host "    Output area: $outputArea" -ForegroundColor Red
        }
        
        if ($Verbose) {
            Write-Host "`n--- Full Output ---" -ForegroundColor Gray
            $output | ForEach-Object { Write-Host "    $_" -ForegroundColor Gray }
            Write-Host ""
        }
    }
    catch {
        Write-Host " ERROR: $_" -ForegroundColor Red
    }
}

Write-Host "`n=== Polygon Simplification Tests ===" -ForegroundColor Cyan
Write-Host ""

Write-Host "--- Simple Cases (polygons with holes) ---" -ForegroundColor Yellow
foreach ($case in $simpleCases) {
    Run-Test -name $case[0] -inputFile $case[1] -target $case[2]
}

if (-not $Quick) {
    Write-Host ""
    Write-Host "--- Lake Cases (target: 99 vertices) ---" -ForegroundColor Yellow
    foreach ($case in $lakeCases) {
        Run-Test -name $case[0] -inputFile $case[1] -target $case[2]
    }
}

Write-Host ""
Write-Host "=== Test Summary ===" -ForegroundColor Cyan
Write-Host "Key validation: Input area must EQUAL Output area" -ForegroundColor White
Write-Host "Quality metric: Lower displacement = better simplification" -ForegroundColor White
Write-Host ""
