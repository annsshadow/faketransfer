/*************************************************************************
	> File Name: fakecommon.c
	> Author: shadowwen-annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Fri 12 Aug 2016 05:44:38 PM HKT
 ************************************************************************/

#include "faketransfer.h"

/*
*copy size string from src to dst
*most copy (size-1)
*more than size will interrupt
*always end by '\0'
*/
size_t strlcpy( char *dst, const char *src, size_t size )
{
    char* d = dst;
    const char* s = src;
    size_t n = size;
    if ( s == 0 || d == 0 )
        return 0;
    /* Copy as many bytes as will fit */
    if (n != 0 && --n != 0)
    {
        do
        {
            if ((*d++ = *s++) == 0)
                break;
        }
        while (--n != 0);
    }
    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0)
    {
        if (size != 0)
            *d = '\0';                /* NUL-terminate dst */
        while (*s++)
            ;
    }
    return(s - src - 1);        /* count does not include NUL */
}


/*
 * append no more than (size - 1 )chars from src to dst
 * retval = strlen(dst)+strlen(src)
 * if retval > size, truncated
*/
size_t strlcat( char* dst, const char* src, size_t size )
{
    char* d = dst;
    const char* s = src;
    size_t n = size;
    size_t dlen;
    if ( s == 0 || d == 0 )
        return 0;
    while (n-- != 0 && *d != '\0')
    {
        d++;
    }
    dlen = d - dst;
    n = size - dlen;
    if (n == 0)
    {
        return(dlen + strlen(s));
    }
    while (*s != '\0')
    {
        if (n != 1)
        {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';
    return(dlen + (s - src));
}


/*
 *success:1
 *fail:-1
 */
int fake_send_string(int socketfd, char *string)
{
    if( string ==  NULL)
    {
        return -1;
    }
    int len =  strlen(string) + 1;
    if( write(socketfd, &len, sizeof(int)) < 0 )
    {
        return -1;
    }
    if( write(socketfd, string, len) < 0 )
    {
        return -1;
    }
    return 1;
}
