extern "C" int for_1_to_n(int);
extern "C" int for_pn_n(int);
extern "C" int for_rev_n(int);

int main()
{
    int a = for_1_to_n(10);   // should get 55
    int b = for_pn_n(25);     // should get 0
    int c = for_rev_n(25);    // should get 0

    if (a == 55 && b == 0 && c == 0)
    {
        return 0;
    }
    return 1;
}