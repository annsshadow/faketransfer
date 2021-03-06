/*************************************************************************
	> File Name: fakecommon.c
	> Author: shadowwen-annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Fri 12 Aug 2016 05:44:38 PM HKT
 ************************************************************************/

#include "faketransfer.h"

/**
 * [fake_strncpy most copy (size -1) chars from source to destination,always end by '/0']
 * @param  {[char *]} dst [point to string destination]
 * @param  {[const char *]} src [point to string source]
 * @param  {[size_t]} size [most copy (size -1) chars]
 * @return {[size_t]} [the length copy from source string, not include the end '/0']
 */
size_t fake_strncpy( char *dst, const char *src, size_t size )
{
    char* d = dst;
    const char* s = src;
    size_t space_length = size;

    //check null or '\0'
    if ( s == 0 || d == 0 )
        return 0;

    //no more than size
    if (space_length != 0 && --space_length != 0)
    {
        do
        {
            if ((*d++ = *s++) == 0)
                break;
        }
        while (--space_length != 0);
    }
    //if Not enough room in dst, add NUL and traverse rest of src
    if (space_length == 0)
    {
        //fill '\0' at the end
        if (size != 0)
            *d = '\0';
        while (*s++)
            ;
    }
    //length not include the end '\0'
    return(s - src - 1);
}


/**
 * [fake_strncat most append string from src to dst, no longer than the length of dst]
 * @param  {[char *]} dst [point to string destination]
 * @param  {[const char *]} src [point to string source]
 * @param  {[size_t]} size [most copy (size -1) chars]
 * @return {[size_t]} [the new length of dst]
 */
size_t fake_strncat( char* dst, const char* src, size_t size )
{
    char* d = dst;
    const char* s = src;
    size_t space_length = size;
    size_t dlen;

    //check null or '/0'
    if ( s == 0 || d == 0 )
        return 0;
    //get the length of dst for now
    while (space_length-- != 0 && *d != '\0')
    {
        d++;
    }
    dlen = d - dst;
    //if the dst is full
    space_length = size - dlen;
    if (space_length == 0)
    {
        return(dlen + strlen(s));
    }
    //check src is '\0' before space_length
    while (*s != '\0')
    {
        if (space_length != 1)
        {
            *d++ = *s;
            space_length--;
        }
        s++;
    }
    *d = '\0';
    return(dlen + (s - src));
}


/**
 * [fake_send_string write some string to connection fd]
 * @param  {[int]} socketfd [the connection fd]
 * @param  {[char *]} string [point to the string ]
 * @return {[int]} [success:1,fail:-1]
 */
int fake_send_string(int socketfd, char *string)
{
    if( string ==  NULL)
    {
        return -1;
    }
    // write string length before string
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
