Copy-Item -Path "..\x64\Debug\HalfFunctionalLang.exe" -Destination "./" -Force

function Compiling {
    param (
        [string]$file_name
    )

    $ass_name = $file_name -replace ".half", ".s"
    $exe_name = "test_" + $file_name -replace ".half", ".exe"
    Write-Host "    Compiling ..."

    if ($file_name -match "main") {
        .\HalfFunctionalLang.exe $file_name -o $ass_name
        clang++ $ass_name -o $exe_name
    } else {
        $cpp_name = "test_" + $file_name -replace ".half", ".cpp"
        .\HalfFunctionalLang.exe $file_name -o $ass_name
        clang++ $ass_name $cpp_name -o $exe_name
    }
    return $exe_name
}

function Run-Test {
    param (
        [string]$exePath
    )

    Write-Host "    Running   ..."
    $exePath = ".\" + $exePath
    & $exePath
    if ($LASTEXITCODE -eq 0) {
        Write-Host "    Success: test passed" -ForegroundColor Green
    } else {
        Write-Host "    Failed: failed with exit code $LASTEXITCODE" -ForegroundColor Red
    }
}

$halfFiles = Get-ChildItem -Path "." -Filter "*.half"
foreach ($file in $halfFiles) {
    Write-Host "Testing $file"
    $exe_name = Compiling $file.Name
    Run-Test $exe_name
}

Remove-Item -path "*.exe" -Force
Remove-Item -path "*.s" -Force
Remove-Item -path "*.o" -Force
Remove-Item -path "*.ilk" -Force
Remove-Item -path "*.pdb" -Force