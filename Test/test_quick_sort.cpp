#include<cstdio>
#include <set>
#include <iostream>

extern "C" int swap(int* arr, int i, int j);
extern "C" int sort(int* arr, int low, int high);
extern "C" int partition(int* arr, int low, int high);

int main(void) {
    int arr[200];
    for (int i = 0; i < 200; i++) {
        arr[i] = 100 - i;
    }
    // shuffle
    for (int i = 0; i < 200; i++) {
        int j = rand() % 200;
        swap(arr, i, j);
    }
    // check swap is correct
    std::set<int> s;
    for (int i = 0; i < 200; i++) {
        s.insert(arr[i]);
    }
    if (s.size() != 200) {
        std::cout << "ERROR::  test_quick_sort fail at swap." << std::endl;
        return 1;
    }

    sort(arr, 0, 199);
    // check
    for (int i = 1; i < 199; i++) {
        if (arr[i] < arr[i - 1]) {
            std::cout << "ERROR::  test_quick_sort fail at sort index: " << i  << "of 200." << std::endl;
            return 1;
        }
    }
    
    return 0;
}