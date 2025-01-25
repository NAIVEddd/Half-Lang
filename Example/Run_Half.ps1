param (
    [string]$file_name = "*.half",
    [string]$sourceDir = "..\x64\Debug",
    [string]$exeName = "HalfFunctionalLang.exe",
    [bool]$delete_files = $true
)
Copy-Item -Path "$sourceDir\$exeName" -Destination ".\" -Force

function Compiling {
    param (
        [string]$file_name
    )

    $ass_name = $file_name -replace ".half", ".s"
    $exe_name = $file_name -replace ".half", ".exe"
    Write-Host "Compiling $file_name"

    if ($file_name -match "main") {
        & .\$exeName $file_name -o $ass_name
        clang++ $ass_name -o $exe_name
    } else {
        $cpp_name = $file_name -replace ".half", ".cpp"
        & .\$exeName $file_name -o $ass_name
        clang++ $ass_name $cpp_name -o $exe_name
    }
    return $exe_name
}

function Run-Exe {
    param (
        [string]$exePath
    )

    $file_name = $exePath -replace ".exe", ".half"
    Write-Host "Running $file_name"
    & $exePath
    if ($LASTEXITCODE -eq 0) {
        Write-Host "    Success: test passed" -ForegroundColor Green
    } else {
        Write-Host "    Failed: failed with exit code $LASTEXITCODE" -ForegroundColor Red
    }
}

function Check-FileExists {
    param (
        [string]$filePath
    )

    if (-not (Get-Item $filePath).Exists) {
        Write-Host "File $filePath does not exist." -ForegroundColor Red
        exit 1
    }
}

Check-FileExists "$exeName"
Check-FileExists "$file_name"
$exe = Compiling $file_name
Check-FileExists "$exe"
Run-Exe $exe



# 删除生成的文件
if ($delete_files) {
    Write-Host "Cleaning up generated files"
    Remove-Item -path ".\*.o" -Force
    Remove-Item -Path ".\*.s" -Force
    Remove-Item -Path ".\*.exe" -Force
    Remove-Item -Path ".\*.gif" -Force
}