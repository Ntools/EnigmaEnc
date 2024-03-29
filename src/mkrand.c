
#include <stdio.h>
#include <stdlib.h>

#include "enge.h"

#define	Err(m)	{ perror(m); exit(1); }

/* Period parameters */  
#define _STVEC_ 624
#define _UNINIT_ 397

#define	H_TABLESIZE	LOWER_MASK  /* hash table size */

#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[_STVEC_]; /* the array for the state vector  */
static int mti=_STVEC_+1; /* mti==_STVEC_+1 means mt[_STVEC_] is not initialized */

unsigned char eng_gear_sel = 0;
unsigned char **eng_gear = NULL;


/* initializes mt[_STVEC_] with a seed */
void init_genrand(unsigned long s)
{
	mt[0] = s & 0xffffffffUL;
	for (mti = 1; mti < _STVEC_; mti++) {
		mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array(unsigned long init_key[], int key_length, unsigned long sd)
{
	int i, j, k;

	init_genrand(sd);
	i = 1; j = 0;
	k = (_STVEC_ > key_length ? _STVEC_ : key_length);
	for (; k; k--) {
		mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
		        + init_key[j] + j; /* non linear */
		mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i >= _STVEC_) { mt[0] = mt[_STVEC_-1]; i=1; }
        if (j >= key_length) j = 0;
	}
	for (k = _STVEC_ - 1; k; k--) {
		mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
		        - i; /* non linear */
		mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
		i++;
		if (i >= _STVEC_) { mt[0] = mt[_STVEC_-1]; i=1; }
    }

	mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
	unsigned long y;
	static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

	if (mti >= _STVEC_) { /* generate _STVEC_ words at one time */
		int kk;

		if (mti == _STVEC_+1)   /* if init_genrand() has not been called, */
			init_genrand(5489UL); /* a default initial seed is used */

		for (kk = 0;kk < _STVEC_ - _UNINIT_;kk++) {
			y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
            mt[kk] = mt[kk + _UNINIT_] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
        for (;kk < _STVEC_ - 1;kk++) {
			y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
			mt[kk] = mt[kk + (_UNINIT_ - _STVEC_)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
		y = (mt[_STVEC_ - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
		mt[_STVEC_ - 1] = mt[_UNINIT_ - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

		mti = 0;
    }

	y = mt[mti++];

    /* Tempering */
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);

	return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(void)
{
	return (long)(genrand_int32()>>1);
}

/* generates a random number on [0,1]-real-interval */
double genrand_real1(void)
{
	return genrand_int32()*(1.0/4294967295.0); 
	/* divided by 2^32-1 */ 
}

/* generates a random number on [0,1)-real-interval */
double genrand_real2(void)
{
	return genrand_int32()*(1.0/4294967296.0); 
	/* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double genrand_real3(void)
{
	return (((double)genrand_int32()) + 0.5)*(1.0/4294967296.0); 
	/* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(void) 
{ 
	unsigned long a = genrand_int32() >> 5, b = genrand_int32() >> 6; 
	return(a * 67108864.0 + b) * (1.0 / 9007199254740992.0); 
} 
/* These real versions are due to Isaku Wada, 2002/01/09 added */

unsigned long hash(char *str) {
	unsigned long v = 0L;

	for ( ; *str; str++) {
		v = ((v << 7)+ *str);
	}
	return v;
}


static int _chk_same(unsigned char ar[], int n, unsigned c)
{
	int i;

	for (i = 0;i < n;++i) {
		if (c == ar[i]) return 0;
	}
	return 1;
}

int randgen(unsigned long sd)
{
	int i, j;
	unsigned char g;
	unsigned long init[4]={0x123, 0x234, 0x345, 0x456}, length = 4;

	eng_gear_sel = 0;
	if (eng_gear == NULL) {
		if ((eng_gear = calloc(256, sizeof(unsigned char *))) == NULL)
			Err("Memory Allocation");
		for(i = 0;i < 256;++i) {
			if ((eng_gear[i] = calloc(256, sizeof(unsigned char))) == NULL)
				Err("Memory Allocation");
		}
	}

	for(i = 0;i < 256;++i) {
		init_by_array(init, length, sd);
		for(j = 0;j < 256;++j) {
			while(1) {
				g = (unsigned char)(genrand_real2() * 256.00);
				if (_chk_same(eng_gear[i], j, g)) break;
			}
#ifdef MAIN
			printf("%02x ", g);
			if (j % 16 == 15) printf("\n");
#endif
			eng_gear[i][j] = g;
		}
#ifdef MAIN
		printf("\n");
#endif
		++sd;
	}
	return 0;
}

#if 0
int randgen(unsigned long sd)
{
	int i;
	unsigned long init[4]={0x123, 0x234, 0x345, 0x456}, length = 4;

	init_by_array(init, length, sd);
#if 0
	printf("1000 outputs of genrand_int32()\n");
	for (i = 0; i < 1000; i++) {
      printf("%10lu ", genrand_int32());
      if (i % 5 == 4) printf("\n");
	}
#endif
	printf("\n1000 outputs of genrand_real2()\n");
	for (i = 0; i < 1000; i++) {
		printf("%10.8f ", genrand_real2());
		if (i%5==4) printf("\n");
	}
	return 0;
}
#endif

#ifdef MAIN
int main(int argc, char *argv[])
{
	unsigned long sd;

	if (argc > 1) sd = hash(argv[1]);
	else sd = 19650218;
	return randgen(sd);
}
#endif
