#include <stdio.h>

class vector
{
public:
    vector(float x, float y);
    float x;
    float y;
    vector add(vector v);
    void dump();
};

vector::vector(float x, float y)
{
    this->x = x;
    this->y = y;
}

vector vector::add(vector v)
{
    this->x = this->x + v.x;
    this->y = this->y + v.y;
    return *this;
}

void vector::dump()
{
    printf("x = %f, y = %f\n", x, y);
}

int f(int a)
{
    return a+1;
}

static int g_int = 0;

int g(int a)
{
    g_int += a;
    return g_int;
}

int main(int argc, char* argv[])
{
    int m;
    int n;

    m = f(1);
    n = g(2);
    printf("f() result = %d, global = %d\n", m, n);

    vector s(1.1, 2.2);
    s.dump();

    return 0;
}
