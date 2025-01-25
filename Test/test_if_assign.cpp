
extern "C" int if_ge_10(int);
extern "C" int if_g_10(int);
extern "C" int if_g_10_v2(int);

int main()
{
    int a = if_ge_10(10);   // should get 1
    int b = if_g_10(10);    // should get 0
    int c = if_g_10_v2(10); // should get 0

    if (a == 0 || b == 1 || c == 1)
    {
        return 1;
    }
    return 0;
}