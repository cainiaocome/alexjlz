unsigned long gcd(unsigned long long a, unsigned long long b)
{
    if ( a < 0 || b < 0 )
        return -1;

    if ( a<b )
        return gcd(b, a);

    if ( b == 0 )
        return a;
    else
        return gcd(b, a%b);
}
