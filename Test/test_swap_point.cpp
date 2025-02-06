struct point
{
    int x;
    int y;
};

struct point3
{
    int x;
    int y;
    int z;
};


extern "C" int swap_p2(point* p, int i, int j);
extern "C" int swap_p3(point3* p, int i, int j);

int main()
{
    point ps[10] = { {1, 2}, {3, 4} };
    int r = swap_p2(ps, 0, 1);
    if (r != 0)
    {
        return 1;
    }

    if (ps[0].x != 3 || ps[0].y != 4)
    {
        return 2;
    }

    if (ps[1].x != 1 || ps[1].y != 2)
    {
        return 3;
    }

    point3 ps3[10] = { {1, 2, 3}, {4, 5, 6} };
    r = swap_p3(ps3, 0, 1);
    if (r != 0)
    {
        return 4;
    }
    if (ps3[0].x != 4 || ps3[0].y != 5 || ps3[0].z != 6)
    {
        return 5;
    }
    if (ps3[1].x != 1 || ps3[1].y != 2 || ps3[1].z != 3)
    {
        return 6;
    }    

    return 0;
}
