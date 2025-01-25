#include <iostream>

extern "C" int square(int a);

int main(void) {
    int arr[200];
    for (int i = 0; i < 200; i++) {
        arr[i] = 200 - i;
    }
    // shuffle
    for (int i = 0; i < 200; i++) {
        int j = rand() % 200;
        std::swap(arr[i], arr[j]);
    }

    for (int i = 0; i < 200; i++) {
        int s = square(arr[i]);
        if (s != arr[i] * arr[i]) {
            std::cout << "ERROR::  test_square fail at square." << std::endl;
            return 1;
        }
    }

    
    return 0;
}