## first class type
- 整数类型
- 浮点类型
- 指针类型
- 数组
- 结构体

;- 函数

### alloca

``` llvm
%ptr = alloca i32                             ; yields ptr
%ptr = alloca i32, i32 4                      ; yields ptr
%ptr = alloca i32, i32 4, align 1024          ; yields ptr
%ptr = alloca i32, align 1024                 ; yields ptr
```
在 LLVM IR 中，`getelementptr` 指令用于计算指针的偏移量，可以用于访问数组元素。对于类似 `std::vector` 的变长数组，通常需要处理动态分配的内存，并使用 `getelementptr` 来计算元素的地址。

### 示例

假设我们有一个动态分配的整数数组，并且我们想要访问数组中的某个元素。以下是如何使用 `getelementptr` 指令来实现这一点的示例。

#### C++ 代码示例

```cpp
#include <vector>
#include <iostream>

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    int value = vec[2];  // 访问第三个元素
    std::cout << value << std::endl;
    return 0;
}
```

#### 对应的 LLVM IR 代码

```llvm
@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; 声明 printf 函数
declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
    ; 分配一个数组
    %vec = alloca [5 x i32], align 16

    ; 初始化数组元素
    %vec_ptr = getelementptr [5 x i32], [5 x i32]* %vec, i32 0, i32 0
    store i32 1, i32* %vec_ptr, align 4
    %vec_ptr1 = getelementptr [5 x i32], [5 x i32]* %vec, i32 0, i32 1
    store i32 2, i32* %vec_ptr1, align 4
    %vec_ptr2 = getelementptr [5 x i32], [5 x i32]* %vec, i32 0, i32 2
    store i32 3, i32* %vec_ptr2, align 4
    %vec_ptr3 = getelementptr [5 x i32], [5 x i32]* %vec, i32 0, i32 3
    store i32 4, i32* %vec_ptr3, align 4
    %vec_ptr4 = getelementptr [5 x i32], [5 x i32]* %vec, i32 0, i32 4
    store i32 5, i32* %vec_ptr4, align 4

    ; 访问第三个元素
    %elem_ptr = getelementptr [5 x i32], [5 x i32]* %vec, i32 0, i32 2
    %value = load i32, i32* %elem_ptr, align 4

    ; 打印值
    %format_str = getelementptr [4 x i8], [4 x i8]* @.str, i32 0, i32 0
    call i32 (i8*, ...) @printf(i8* %format_str, i32 %value)

    ; 返回 0
    ret i32 0
}
```

### 解释

1. **分配一个数组**：
    ```llvm
    %vec = alloca [5 x i32], align 16
    ```
    - 在栈上分配一个包含 5 个 `i32` 元素的数组。
    vec 包含的信息：{base 寄存器，偏移量}(栈起始地址)，type: ptr of [5 x i32] *
        alloca 的结果应该保存在symbol中

2. **初始化数组元素**：
    ```llvm
    %vec_ptr = getelementptr [5 x i32], [5 x i32]* %vec, i32 0, i32 0
    store i32 1, i32* %vec_ptr, align 4
    ```
    - 使用 `getelementptr` 计算数组中第一个元素的地址，并将值 `1` 存储到该地址。
    - 类似地，初始化数组中的其他元素。
    vec_ptr 包含：{base 寄存器，偏移量(+ 0 * sizeof(5 * i32) + 0 * sizeof(i32))}, type

3. **访问第三个元素**：
    ```llvm
    %elem_ptr = getelementptr [5 x i32], [5 x i32]* %vec, i32 0, i32 2
    %value = load i32, i32* %elem_ptr, align 4
    ```
    - 使用 `getelementptr` 计算数组中第三个元素的地址，并使用 `load` 指令加载该元素的值。
        load 从symbol中获取地址信息，把结果存到寄存器中（显然类型只能是基本类型--小于8bytes）
        value 包含：类型，寄存器名

4. **打印值**：
    ```llvm
    %format_str = getelementptr [4 x i8], [4 x i8]* @.str, i32 0, i32 0
    call i32 (i8*, ...) @printf(i8* %format_str, i32 %value)
    ```
    - 使用 `getelementptr` 获取格式字符串的地址，并调用 `printf` 函数打印值。
    call 结果包含：寄存器

5. **返回 0**：
    ```llvm
    ret i32 0
    ```
    - 返回 `0` 以结束程序。

Address
{
    Type type
    Temp::Label base
    size_t offset
}
Register
{
    Type type
    Temp::Label reg
}
Value = Address | Register

### 总结

在 LLVM IR 中，可以使用 `getelementptr` 指令来计算变长数组（类似 `std::vector`）中元素的地址。通过这种方式，可以高效地访问和操作数组中的元素。上述示例展示了如何在 LLVM IR 中使用 `getelementptr` 指令来访问和操作动态分配的数组。


### load & store
load 接受一个指针，返回指针指向的类型

``` llvm
%ptr = alloca i32                               ; yields ptr
store i32 3, ptr %ptr                           ; yields void
%val = load i32, ptr %ptr                       ; yields i32:val = i32 3
```

### getelementptr
<result> = getelementptr <ty>, ptr <ptrval>{, <ty> <idx>}*
``` llvm
%struct.RT = type { i8, [10 x [20 x i32]], i8 }
%struct.ST = type { i32, double, %struct.RT }

define ptr @foo(ptr %s) {
entry:
  %arrayidx = getelementptr inbounds %struct.ST, ptr %s, i64 1, i32 2, i32 1, i64 5, i64 13
  ret ptr %arrayidx
}
```
``` llvm
%aptr = getelementptr {i32, [12 x i8]}, ptr %saptr, i64 0, i32 1
%vptr = getelementptr {i32, <2 x i8>}, ptr %svptr, i64 0, i32 1, i32 1
%eptr = getelementptr [12 x i8], ptr %aptr, i64 0, i32 1
%iptr = getelementptr [10 x i32], ptr @arr, i16 0, i16 0
```
``` llvm
; All arguments are vectors:
;   A[i] = ptrs[i] + offsets[i]*sizeof(i8)
%A = getelementptr i8, <4 x i8*> %ptrs, <4 x i64> %offsets

; Add the same scalar offset to each pointer of a vector:
;   A[i] = ptrs[i] + offset*sizeof(i8)
%A = getelementptr i8, <4 x ptr> %ptrs, i64 %offset

; Add distinct offsets to the same pointer:
;   A[i] = ptr + offsets[i]*sizeof(i8)
%A = getelementptr i8, ptr %ptr, <4 x i64> %offsets

; In all cases described above the type of the result is <4 x ptr>
```

### icmp
<result> = icmp <cond> <ty> <op1>, <op2>   ; yields i1 or <N x i1>:result
``` llvm
<result> = icmp eq i32 4, 5          ; yields: result=false
<result> = icmp ne ptr %X, %X        ; yields: result=false
<result> = icmp ult i16  4, 5        ; yields: result=true
<result> = icmp sgt i16  4, 5        ; yields: result=false
<result> = icmp ule i16 -4, 5        ; yields: result=false
<result> = icmp sge i16  4, 5        ; yields: result=false
```

### phi
<result> = phi [fast-math-flags] <ty> [ <val0>, <label0>], ...
``` llvm

```