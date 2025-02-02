extern "C" int swap(int*, int, int);

int main()
{
    int array[10] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
    int a = 1;
    int b = 2;
    int c = swap(array, a, b);
    if (array[1] != 8 || array[2] != 9 || c != 0)
    {
        return 1;
    }
    return 0;
}