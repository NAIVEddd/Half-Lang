 Copy-Item -Path "..\x64\Debug\HalfFunctionalLang.exe" -Destination "./" -Force

.\HalfFunctionalLang.exe .\quick_sort.half -o .\quick_sort.s
clang++ quick_sort.s .\test_quick_sort.cpp -o test_quick_sort.exe
.\test_quick_sort.exe
 
 .\HalfFunctionalLang.exe .\square.half -o square.s
 clang++ .\square.s .\test_square.cpp -o test_square.exe
 .\test_square.exe