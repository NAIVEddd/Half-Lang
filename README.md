# Half Lang Compiler

## 项目简介

HalfFunctionalLang Compiler 是一个简单的编译器项目，旨在构建一种新的语言"Half"。此项目读入Half源文件，并生成x86-64机器代码。该项目使用类似 LLVM 的IR，进行优化和代码生成。


## 构建和运行
### 依赖项
- VS 2022
### 构建项目

1. 克隆项目到本地：

    ```sh
    git clone https://github.com/naiveddd/HalfLang.git
    cd HalfLang
    ```

## 示例

```half
function square(n int) =
    let x = n * n
    x
end
```

### Usage
```sh
Half.exe hello.half -o half.s
clang -c half.s     # compile asm to .o
```

## 许可证
该项目使用 MIT 许可证。详情请参阅 LICENSE 文件。