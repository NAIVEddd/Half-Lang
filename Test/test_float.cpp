#include <stdio.h>
extern "C" float add_float(float a, float b);

extern "C" float abs(float a);
extern "C" int abs_int(int a);

int main()
{
    if (abs_int(-1) != 1)
    {
        return -1;
    }

    float a = 0.5;
    float r = add_float(a, a);

    if (r != 1.0)
    {
        return 1;
    }

    int x = (int)r;
    if (x != 1)
    {
        return 1;
    }

    a = 15.0f;
    r = add_float(a, a);
    x = (int)r;
    if (x != 30)
    {
        return 2;
    }

    a = -15.0f;
    r = abs(a);
    //printf("abs(-15.0) = %f\n", r);
    if (r != 15.0)
    {
        return (int)r;
    }

    return 0;
}