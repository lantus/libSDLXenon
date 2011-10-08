#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <time/time.h>
#include <xenon_smc/xenon_smc.h>



void usleep(int s) {
    udelay(s);
}

int stat(const char * __restrict path, struct stat * __restrict buf) {
    int fd = -1;
    fd = open(path, O_RDONLY);

    if (fd) {
        return fstat(fd, buf);
    }
    return ENOENT; // file doesn't exist
}
#if 1
// 22 nov 2005 
#define	RTC_BASE	1132614024UL//1005782400UL

int gettimeofday(struct timeval * tp, void * tzp) {
    unsigned char msg[16] = {0x04};
    unsigned long msec;
    unsigned long sec;

    xenon_smc_send_message(msg);
    xenon_smc_receive_message(msg);

    msec = msg[1] | (msg[2] << 8) | (msg[3] << 16) | (msg[4] << 24) | ((unsigned long) msg[5] << 32);

    //      printf("smc ret : %d\r\n", msec);

    sec = msec / 1000;

    tp->tv_sec = sec + RTC_BASE;

    msec -= sec * 1000;

    tp->tv_usec = msec * 1000;

    //printf("s:%d - %ms:%d\r\n", tp->tv_sec, tp->tv_usec);

    //printf("%s\r\n",ctime(&tp->tv_sec));
    
    return 0;
}
#endif

#if 0
#include <altivec.h>
#define vector_s16_t vector signed short
#define vector_u16_t vector unsigned short
#define vector_s8_t vector signed char
#define vector_u8_t vector unsigned char
#define vector_s32_t vector signed int
#define vector_u32_t vector unsigned int
#define MMREG_SIZE 16

#define SMALL_MEMCPY(to, from, len)                                         \
{                                                                           \
    unsigned char * end = to + len;                                         \
    while( to < end )                                                       \
    {                                                                       \
        *to++ = *from++;                                                    \
    }                                                                       \
}

void * fast_memcpy(void * _to, const void * _from, size_t len) {
    void * retval = _to;
    unsigned char * to = (unsigned char *) _to;
    unsigned char * from = (unsigned char *) _from;

    if (len > 16) {
        /* Align destination to MMREG_SIZE -boundary */
        register unsigned long int delta;

        delta = ((unsigned long) to)&(MMREG_SIZE - 1);
        if (delta) {
            delta = MMREG_SIZE - delta;
            len -= delta;
            SMALL_MEMCPY(to, from, delta);
        }

        if (len & ~(MMREG_SIZE - 1)) {
            vector_u8_t perm, ref0, ref1, tmp;

            perm = vec_lvsl(0, from);
            ref0 = vec_ld(0, from);
            ref1 = vec_ld(15, from);
            from += 16;
            len -= 16;
            tmp = vec_perm(ref0, ref1, perm);
            while (len & ~(MMREG_SIZE - 1)) {
                ref0 = vec_ld(0, from);
                ref1 = vec_ld(15, from);
                from += 16;
                len -= 16;
                vec_st(tmp, 0, to);
                tmp = vec_perm(ref0, ref1, perm);
                to += 16;
            }
            vec_st(tmp, 0, to);
            to += 16;
        }
    }

    if (len) {
        SMALL_MEMCPY(to, from, len);
    }

    return retval;
}

#endif
