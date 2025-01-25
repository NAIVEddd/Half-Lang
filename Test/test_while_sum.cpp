extern "C" int while_1_to_n(int);
extern "C" int while_n_to_1(int);

int main()
{
    int res = while_1_to_n(10);
    if (res != 45)
    {
        return 1;
    }
    res = while_n_to_1(10);
    if (res != 54)
    {
        return 1;
    }
    return 0;
}