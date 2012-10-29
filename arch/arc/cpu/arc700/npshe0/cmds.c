
#include <common.h>
#include <command.h>
#include <net.h>
#include "setjmp.h"
#include "iomap.h"

#define PREBOOT_SIZE	0x10000 /* 64k */
#define UBOOT_SIZE	0x80000 /* 0.5 MB */
#define KERNEL_SIZE	0x500000 /* 5 MB */

#define START_RAM        CONFIG_SYS_MEMTEST_START
#define RAM_SIZE         0x20000000
#define RAM_SIZE4        (RAM_SIZE/4)       /* 0x08000000 */
#define RAM_TEST_SIZE    (CONFIG_SYS_MEMTEST_END - CONFIG_SYS_MEMTEST_START)
#define RAM_TEST_SIZE4   (RAM_TEST_SIZE/4)  /* 0x07FBC000 */

#define M_CORE_VEC  0x81000000
#define VALUE1  0x5a5a5a5a
#define BASE_RANDOM  0x148d3a2b
#define N_SAMPLES  128

extern int rt_bwl(void *start);
extern unsigned long rtsc32(void);
extern unsigned long long rtsc64(void);
extern int swapAsm(int startValue,int *vec,int loops);
extern int dummyCall(int value);
extern int txFifoStatus(void);
extern void special_serial(char c,int *vec);
extern void serial_putc (const char c);

int validAux[] = { 0x000,0x002,0x003,0x004,0x005,0x006,0x00a,0x00b,0x00c,0x00f,0x011,0x01a,0x01b,0x01c,0x01d,
				   0x01e,0x021,0x022,0x023,0x025,0x041,0x043,0x044,0x045,0x046,0x048,0x058,0x059,0x05a,0x05b,
				   0x05c,0x100,0x101,0x102,0x200,0x201,0x262,0x400,0x401,0x402,0x403,0x404,0x405,0x406,0x407,
				   0x409,0x40a,0x40b,0x40c,0x40d,0x40e,0x410,0x413,0x414,0x416,0x418,-1};

typedef struct {
	int   	param;
	void   *ptr;
}CORE_BOOT;

CORE_BOOT *coreBootParams = (CORE_BOOT *)0xFFFFFFF8;
static jmp_buf  bufJump;

void doResetNPSHE0(void)
{
	void (*reset)();
	reset = (void (*)())0xFFFFE000;
	(*reset)();
}

static int rt_roll01(void *start,int mode)
{
    unsigned long vrefDef;
    unsigned long vref;
    unsigned long base;
    volatile unsigned long *ptr;
    int i,j,n;

    if ( mode == 1 ) {
        vrefDef = 0xFFFFFFFE;
        base = 0xFFFFFFFF;
    } else {
        vrefDef = 0x00000001;
        base = 0;
    }

    ptr = (volatile unsigned long *)start;
    for ( n = 0 ; n < 8 ; n++ ) {
        vref = vrefDef;
        for ( i = 0 ; i < 32 ; i++ ) {
            for ( j = 0 ; j < 8 ; j++ , ptr++ )
                *ptr = (j == n) ? vref : base;
            vref <<= 1;
            vref += mode;;
        }
    }

    ptr = (volatile unsigned long *)start;
    for ( n = 0 ; n < 8 ; n++ ) {
        vref = vrefDef;
        for ( i = 0 ; i < 32 ; i++ ) {
            for ( j = 0 ; j < 8 ; j++ , ptr++ ) {
                if ( *ptr != ((j == n) ? vref : base) )
                    return (0x40000000 | (n <<16) | (i << 8) | j);
            }
            vref <<= 1;
            vref += mode;
        }
    }

    return 0;
}

static int rt_overlap(void *start)
{
    int long baseValue = 0x159d036c;
    int long deltaValue = 0x05af159d;
    int value;
    int i;
    volatile int long *ptr;

    value = baseValue + deltaValue;
    for ( i = 0x10 ; i < RAM_TEST_SIZE ; i <<= 1 , value += deltaValue ) {
        ptr = (volatile int long *)(((int)start) + i);
        *ptr = value;
    }

    ptr = (volatile int long *)((int)start);
    *ptr = value;
    value = baseValue + deltaValue;

    for ( i = 0x10 ; i < RAM_TEST_SIZE ; i <<= 1 , value += deltaValue ) {
        ptr = (volatile int long *)(((int)start) + i);
        if (*ptr != value)
             return (int)ptr;
    }

    ptr = (volatile int long *)((int)start);
    if (*ptr != value) {
        return (int)ptr;
    }

    return 0;
}

static int inline updateRandom27(int value)
{
    int k;
    int retValue = (value << 1) & 0x07FFFFFE;
    static const int randTlb[16] = {0,1,1,0, 1,0,0,1, 1,0,0,1, 0,1,1,0};

    value >>= 21;
    k = (value & 0x038) >> 3;
    if (value & 0x001) k += 8;

    return retValue | randTlb[k];
}

static int rt_fill(void *start,int value1)
{
    int i;
    int offset;
    int long *ptr = (int long *)start;
    int value2 = value1 ^ 0xFFFFFFFF;

    printf("start part 1\n");
    for ( i = 0 ; i < RAM_TEST_SIZE4 ; i++, ptr++ )
        *ptr = value1;

    printf("end part 1, start part 2\n");
    ptr = (int long *)start;
    if ( *ptr != value1 )
        return (int)ptr;

    *ptr = value2;
    offset = BASE_RANDOM;

    for ( i = 1 ; i < RAM_SIZE4 ; i++ ) {
#if RAM_SIZE == RAM_TEST_SIZE
        offset = updateRandom27(offset);
        ptr = (int long *)(((int)start)+(offset<<2));
#else
        offset = (updateRandom27(offset >> 2 ) ) << 2;
        if ( offset >= RAM_TEST_SIZE ) continue;
        ptr = (int long *)(((int)start)+offset);
#endif
        if ( *ptr != value1 )
            return (int)ptr;
        *ptr = value2;
    }

    printf("end part 2, start part 3\n");
    ptr = (int long *)start;

    for ( i = 0 ; i < RAM_TEST_SIZE4 ; i++, ptr++ ) {
        if ( *ptr != value2 )
            return (int)ptr | 0x80000000;
        *ptr = 0;
    }

    printf("end part 3,start part 4\n");
    value1 = 0;
    ptr = (int long *)start;

    for ( i = 0 ; i < RAM_TEST_SIZE4 ; i++, ptr++ )
        value1 |= *ptr;

    if (value1 != 0)
    	return -1;
    printf("end part 4\n");

    return 0;
}

static int inline updateRandom31(int value)
{
    int retValue = (value << 1) & 0x7FFFFFFE;
    int k;

    k = ( value & 0x40000000 ) ? 1 : 0;
    if ( value & 0x08000000 ) k++;

    return (retValue | (k & 1));
}

static int rt_random(void *start)
{
    int i;
    int long *ptr = (int long *)start;
    int value = BASE_RANDOM;
    for ( i = 0 ; i < RAM_TEST_SIZE4 ; i++, ptr++ ) {
        value = updateRandom31(value);
        *ptr = value;
    }

    ptr = (int long *)start;
    value = BASE_RANDOM;

    for ( i = 0 ; i < RAM_TEST_SIZE4 ; i++, ptr++ ) {
        value = updateRandom31(value);
        if (*ptr != value)
            return (int)ptr;
    }

    return 0;
}

static int timeStampTest(void)
{
    int vec[N_SAMPLES+1];
    int i;
    long long cTime;
    int lsb,msb;

    for ( i = 0 ; i < (N_SAMPLES+1) ; i++ )
        vec[i] = rtsc32();

    i = vec[N_SAMPLES]-vec[0];
    printf("test time %d (0x%08x) clocks for %d loops. Counter clock = %dHz.\n",i,i,N_SAMPLES,CONFIG_SYS_HZ);
    cTime = rtsc64();
    cTime++;

    msb = (int)(cTime >> 32);
    lsb = (int)(cTime & 0xFFFFFFFF);

    printf("Current counter state is: 0x%x%08x\n",msb,lsb);
    return 0;
}

static int sigmaData(int *start,int len)
{
    int i,retValue;
    for ( retValue = i = 0 ; i < len ; i++ , start++ )
    	retValue += *start;

    return retValue;
}

static void memset4(int *start,int len,int value)
{
    int i;
    for ( i = 0 ; i < len ; i++ , start++ )
    	*start = value;
    return ;
}

static int sigmaDataWr(int *start,int len,int value)
{
    int i,retValue;
    for ( retValue = i = 0 ; i < len ; i++ , start++ ) {
        retValue += *start;
        *start = value;
    }
    return retValue;
}

static void minAvrMax(int size,int *vec,int len)
{
    int min = vec[0];
    int max = vec[0];
    int avr = vec[0];
    int i;

    for ( i = 1 ; i < len ; i++ ) {
        avr += vec[i];
        if ( min > vec[i] ) min = vec[i];
        if ( max < vec[i] ) max = vec[i];
    }

    avr += (len >> 1);
    avr /= len;
    printf("size %5d  %7d - %7d - %7d\n",size,min,avr,max);
}

static void printExchangeVec(int mask)
{
    int *ptr = (int *)M_CORE_VEC;
    int i;

    for ( i = 0 ; i < 100000 ; i++ ) // wait to all cores
        i = dummyCall(i);

    REG_CPU_HALT_CTL &= ~(mask & 0x0e);
    for ( i = 0 ; i < 8 ; i++ )
        printf("%08x ",ptr[i]);

    printf("\n");
}

static void activeCores(int mask)
{
    if ( ( mask & 0x0e ) != 0 ) {
        REG_CPU_RST_CTL |= mask & 0x0e;
        REG_CPU_RST_CTL &= ~(mask & 0x0e);
        REG_CPU_HALT_CTL |= mask & 0x0e;
    }

    if ( ( mask & 0x01 ) != 0 ) {
        int *ptr = (int *)M_CORE_VEC;
        if ( mask & 0x10 ) {
            ptr[0+4] = swapAsm(ptr[0+4],ptr,50001);
        } else {
            void (*fp)(void) = NULL;
            fp();
        }
    }
}

static void initExchangeVec(void)
{
    int *ptr = (int *)M_CORE_VEC;
    int i;
    int k = 0x01010101;

    for ( i = 0 ; i < 8 ; i++ , k <<= 1 )
        ptr[i] = k;
}

static int auxRegRead(int i)
{
    switch (i) {
        case 0x000:  return read_new_aux_reg(0x000);
        case 0x002:  return read_new_aux_reg(0x002);
        case 0x003:  return read_new_aux_reg(0x003);
        case 0x004:  return read_new_aux_reg(0x004);
        case 0x005:  return read_new_aux_reg(0x005);
        case 0x006:  return read_new_aux_reg(0x006);
        case 0x00a:  return read_new_aux_reg(0x00a);
        case 0x00b:  return read_new_aux_reg(0x00b);
        case 0x00c:  return read_new_aux_reg(0x00c);
        case 0x00f:  return read_new_aux_reg(0x00f);
        case 0x011:  return read_new_aux_reg(0x011);
        case 0x01a:  return read_new_aux_reg(0x01a);
        case 0x01b:  return read_new_aux_reg(0x01b);
        case 0x01c:  return read_new_aux_reg(0x01c);
        case 0x01d:  return read_new_aux_reg(0x01d);
        case 0x01e:  return read_new_aux_reg(0x01e);
        case 0x021:  return read_new_aux_reg(0x021);
        case 0x022:  return read_new_aux_reg(0x022);
        case 0x023:  return read_new_aux_reg(0x023);
        case 0x025:  return read_new_aux_reg(0x025);
        case 0x041:  return read_new_aux_reg(0x041);
        case 0x043:  return read_new_aux_reg(0x043);
        case 0x044:  return read_new_aux_reg(0x044);
        case 0x045:  return read_new_aux_reg(0x045);
        case 0x046:  return read_new_aux_reg(0x046);
        case 0x048:  return read_new_aux_reg(0x048);
        case 0x058:  return read_new_aux_reg(0x058);
        case 0x059:  return read_new_aux_reg(0x059);
        case 0x05a:  return read_new_aux_reg(0x05a);
        case 0x05b:  return read_new_aux_reg(0x05b);
        case 0x05c:  return read_new_aux_reg(0x05c);
        case 0x100:  return read_new_aux_reg(0x100);
        case 0x101:  return read_new_aux_reg(0x101);
        case 0x102:  return read_new_aux_reg(0x102);
        case 0x200:  return read_new_aux_reg(0x200);
        case 0x201:  return read_new_aux_reg(0x201);
        case 0x262:  return read_new_aux_reg(0x262);
        case 0x400:  return read_new_aux_reg(0x400);
        case 0x401:  return read_new_aux_reg(0x401);
        case 0x402:  return read_new_aux_reg(0x402);
        case 0x403:  return read_new_aux_reg(0x403);
        case 0x404:  return read_new_aux_reg(0x404);
        case 0x405:  return read_new_aux_reg(0x405);
        case 0x406:  return read_new_aux_reg(0x406);
        case 0x407:  return read_new_aux_reg(0x407);
        case 0x409:  return read_new_aux_reg(0x409);
        case 0x40a:  return read_new_aux_reg(0x40a);
        case 0x40b:  return read_new_aux_reg(0x40b);
        case 0x40c:  return read_new_aux_reg(0x40c);
        case 0x40d:  return read_new_aux_reg(0x40d);
        case 0x40e:  return read_new_aux_reg(0x40e);
        case 0x410:  return read_new_aux_reg(0x410);
        case 0x412:  return read_new_aux_reg(0x412);
        case 0x413:  return read_new_aux_reg(0x413);
        case 0x414:  return read_new_aux_reg(0x414);
        case 0x416:  return read_new_aux_reg(0x416);
        case 0x418:  return read_new_aux_reg(0x418);
    default:
        break;
    }
    return 0xAAAAAAAA;
}

static void printAuxRegFrom(int from)
{
    int i;
    int k;
    for ( i = 0 ; i < 300 ; i++ ) {
        if ( ( validAux[i] < 0 ) || ( validAux[i] >= from ) ) break;
    }
    if ( validAux[i] < 0 ) {
        printf(" No Reg after %03x\n",from);
        return;
    }
    k = 0;
    for ( ; validAux[i] >= 0 ; i++ ) {
        printf(" %03x - " , validAux[i] );
        while(serial_tstc() != 0);
        printf("%08x",auxRegRead(validAux[i]));
        k++;
        if ( k >= 4 ) {
            printf("\n");
            k = 0 ;
        }
    }
    printf("\n");
}

static void coreStartRoutine(jmp_buf bufJump)
{
	int k;
	printf(" identity = 0x%08x\n",auxRegRead(4));
	/* wait 0.1sec.*/
	k = rtsc32();
	while ( (rtsc32()-k) < (CONFIG_SYS_HZ/10));
	longjmp(bufJump,1);
	return;
}
static int do_mem_test ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int k;
    int n;
    int i;

    if ( argc < 2 ) {
		cmd_usage(cmdtp);
		return 1;
    }

    n = simple_strtoul(argv[1], NULL, 16);

    switch (n) {

        case 0x1:
             for ( i = 0 ; i < 1000000 ; i++ ) {
                 k = rt_bwl((void *)(START_RAM+(i<<6)));
                 if ( k )
                	 break;
             }
             break;

        case 0x2:
             k = rt_roll01((void *)START_RAM,1);
             if ( k != 0 ) {
                 k |= 0x80000000;
                 break;
             }
             k = rt_roll01((void *)(START_RAM+0x00010000),0);
             break;

        case 0x3:
             k = rt_overlap((void *)START_RAM);
             break;

        case 0x4:
             k = rt_fill((void *)START_RAM,VALUE1);
             break;

        case 0x5:
             k = rt_random((void *)START_RAM);
             break;

        case 0x6:
             k = timeStampTest();
             break;

        case 0x7:
             {
                int l,z,t;
                int vec[17];
                for ( l = 0x100 ; l <= 0x10000 ; l += 0x100 ) {
                    z = l >> 2;
                    for ( t = 0 ; t < 17 ; t++ ) {
                        sigmaData((int *)START_RAM,z);
                        n = rtsc32();
                        sigmaData((int *)START_RAM,z);
                        n = rtsc32() - n;
                        vec[t] = n;
                    }
                    minAvrMax(l,&vec[1],16);
                }

                for ( l = 0x100 ; l <= 0x10000 ; l += 0x100 ) {
                    z = l >> 2;
                    for ( t = 0 ; t < 17 ; t++ ) {
                        memset4((int *)START_RAM,z,t);
                        n = rtsc32();
                        memset4((int *)START_RAM,z,t);
                        n = rtsc32() - n;
                        vec[t] = n;
                    }
                    minAvrMax(l,&vec[1],16);
                }

                for ( l = 0x100 ; l <= 0x10000 ; l += 0x100 ) {
                    z = l >> 2;
                    for ( t = 0 ; t < 17 ; t++ ) {
                        sigmaDataWr((int *)START_RAM,z,t);
                        n = rtsc32();
                        sigmaDataWr((int *)START_RAM,z,t);
                        n = rtsc32() - n;
                        vec[t] = n;
                    }
                    minAvrMax(l,&vec[1],16);
                }
             }
             k = 0;
             n = 7;
             break;

        case 0x8:
             k = simple_strtoul(argv[2], NULL, 16);
             switch ( k ) {
             case 0:
                  printf("disable dcache & icache\n");
                  dcache_disable();
                  icache_disable();
                  break;
             case 1:
                  printf("enable dcache & disable icache \n");
                  dcache_enable();
                  icache_disable();
                  k = 0;
                  break;
             case 2:
                  printf("disable dcache & enable icache \n");
                  dcache_disable();
                  icache_enable();
                  k = 0;
                  break;
             case 3:
                  printf("enable dcache & icache\n");
                  dcache_enable();
                  icache_enable();
                  k = 0;
                 break;
             default:
                  k = -1;
                  break;
             }
             break;

        case 0x9:
			 k = 0;
			 if ( argc >= 3 )
				 k = simple_strtoul(argv[2], NULL, 16);

			 printAuxRegFrom(k);
			 k = 0;
			 break;

        case 0xa:
            k = 0x11;
            if ( argc >= 3 )
                k = simple_strtoul(argv[2], NULL, 16);

            initExchangeVec();
            activeCores(k);
            printExchangeVec(k);
            k = 0;

            break;
        case 0xb:
        	if ( argc < 3 ) {
        		printf(" add core number .\n");
        		k = -1;
        		break;
        	}
        	n = auxRegRead(4);
        	n = (n >> 8) & 0x0FF;
        	k = simple_strtoul(argv[2], NULL, 16);
        	if ( ( ( k & 0xFFFFFFFC ) != 0 ) || ( k == n) ) {
        		printf(" Valid core numbers are 0-3 but not core %d\n",n);
        		n = 0xb;
        		k = -1;
        		break;
        	}
        	coreBootParams->param = (int)bufJump;
        	coreBootParams->ptr = coreStartRoutine;
        	if ( setjmp(bufJump) ) {
/* run from other core*/
        		printf("now run from 0x%08x\n",auxRegRead(4));
    	    	n = 0xb;
    	        k = 0;
    	        break;
        	}
        	k = 1 << k;
        	REG_CPU_HALT_CTL &= ~(k);
            REG_CPU_RST_CTL |= k;
            REG_CPU_RST_CTL &= ~(k);
            REG_CPU_HALT_CTL |= k;
/* wait 0.01sec.*/
            k = rtsc32();
            while ( (rtsc32()-k) < (CONFIG_SYS_HZ/100));
/* make reset to prev. core */
            for ( k = 0 ; k != 1 ; k += 2 ) k = dummyCall(k);
            n = 1 << n;
        	REG_CPU_HALT_CTL &= ~(n);
            REG_CPU_RST_CTL |= n;
    		printf("ERROR from 0x%08x\n",auxRegRead(4));
    		n = 0xb;
            k = 0;
            break;
         default:
        	 cmd_usage(cmdtp);
        	 return 1;
    }
    if ( k != 0 )
         printf("Test %d return error 0x%08x\n",n,k);
    else
         printf("Test %d pass!\n",n);

    return 0;
}

#include "environment.h"

static int do_resetenv(cmd_tbl_t *cmdtp, int flag,int argc, char *argv[])
{
	char ipaddr[32];
	char etheddr[32];


	if (argc > 2)
		goto usage;

	strcpy(ipaddr,getenv("ipaddr"));
	strcpy(etheddr,getenv("ethaddr"));

	if (argc == 2) {
		if (!argv[1] && (strcmp(argv[1],"full") != 0 ))
			goto usage;
	}

	set_default_env(NULL);

	if (argc == 1) {
		setenv ("ipaddr", ipaddr);
		setenv ("ethaddr", etheddr);
	}

	 if (saveenv()) {
		 printf("Writing env to flash failed\n");
		 return 1;
	 }

	return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

static int do_upkrn_sz(cmd_tbl_t *cmdtp, int flag,int argc, char *argv[]){
	int *load_addr;
	char *load_address_string;
	char kernel_size[32];

	if (argc > 1)
		goto usage;

	load_address_string = getenv("loadaddr");
	load_addr = (int*)simple_strtoul(load_address_string, NULL, 16);

	if (*load_addr == IH_MAGIC) {
		sprintf(kernel_size,"%lx",load_addr[3] + sizeof(image_header_t));
		setenv ("krn_size", kernel_size);
		return 0;
	} else {
		printf("file in loadaddr is not uImage\n");
	}
usage:
	cmd_usage(cmdtp);
	return 1;
}

static int do_file_load(char *file_name_var, char* mode)
{
	char *file_name;
	char cmdbuf[CONFIG_SYS_CBSIZE];

	if ( !file_name_var || !(file_name_var[0]))
		goto usage;

	file_name = getenv(file_name_var);
	if (!file_name) {
		printf("%s variable is not defined\n",file_name_var);
		goto usage;
	}

	if ( (mode != NULL) && (strcmp(mode,"serial") == 0) ) {
		strcpy (cmdbuf, "loady");
	} else {
		strcpy (cmdbuf, "tftpboot ");
		strcat (cmdbuf, file_name);
	}

	if (run_command (cmdbuf, 0) < 0) /* tftpboot ${file_name} */
		goto usage;

	return 0;
usage:
	printf("Usage: do_file_load [file_name] \n");
	return 1;

}

static int do_preboot_write(void)
{
	int preboot_sf_size;
	int preboot_sf_location;
	int preboot_sector_offset;
	int load_addr;
	char *preboot_offset;
	char *preboot_file_size;
	char *preboot_load_address;
	char sector_location[32];
	char spi_sector_location[32];
	char spi_sector_size[32];
	char cmdbuf[CONFIG_SYS_CBSIZE];

	preboot_offset = getenv("preboot_offs");
	preboot_file_size = getenv("filesize");
	preboot_load_address = getenv("loadaddr");

	/* Check environment variables */
	if (!preboot_offset || !preboot_file_size || !preboot_load_address) {
		printf("Environment variables are not defined:\n");
		printf("preboot_offs:%s\n",preboot_offset);
		printf("filesize:%s\n",preboot_file_size);
		printf("loadaddr:%s\n",preboot_load_address);
		goto usage;
	}

	/* Check file size */
	preboot_sf_size = simple_strtoul(preboot_file_size, NULL, 16);
	if (preboot_sf_size > PREBOOT_SIZE) {
		printf("file size is larger then %x\n",PREBOOT_SIZE);
		goto usage;
	}

	if (run_command ("sf probe 0", 0) < 0) /* sf probe 0 */
		goto usage;

	load_addr = simple_strtoul(preboot_load_address, NULL, 16);
	/* Write the sector after the preboot (64k) */
	sprintf(sector_location,"%x",load_addr + PREBOOT_SIZE);
	preboot_sf_location = simple_strtoul(preboot_offset, NULL, 16);
	/* Read from the start of the sector */
	preboot_sector_offset = preboot_sf_location - (preboot_sf_location % CONFIG_ENV_SECT_SIZE);

	sprintf(spi_sector_location,"%x",preboot_sector_offset);
	sprintf(spi_sector_size,"%x",CONFIG_ENV_SECT_SIZE);

	strcpy(cmdbuf,"sf read ");
	strcat(cmdbuf,sector_location);
	strcat(cmdbuf," ");
	strcat(cmdbuf,spi_sector_location);
	strcat(cmdbuf," ");
	strcat(cmdbuf,spi_sector_size);

	if (run_command (cmdbuf, 0) < 0) /* sf read ${load_addr}+64k ${preboot_sec_offs} ${preboot_size} */
		goto usage;

	/* initialize  the 64k of the preboot */
	memset((void*)(load_addr + PREBOOT_SIZE + PREBOOT_SIZE),0xFF,PREBOOT_SIZE);
	/* copy preboot to sector */
	memcpy((void*)(load_addr + PREBOOT_SIZE + PREBOOT_SIZE),(void*)load_addr,preboot_sf_size);

	strcpy(cmdbuf,"sf erase ");
	strcat(cmdbuf,spi_sector_location);
	strcat(cmdbuf," ");
	strcat(cmdbuf,spi_sector_size);

	if (run_command (cmdbuf, 0) < 0) /* sf erase ${preboot_offs} 1 block */
		goto usage;

	strcpy(cmdbuf,"sf write ");
	strcat(cmdbuf,sector_location);
	strcat(cmdbuf," ");
	strcat(cmdbuf,spi_sector_location);
	strcat(cmdbuf," ");
	strcat(cmdbuf,spi_sector_size);

	if (run_command (cmdbuf, 0) < 0) /* sf  write ${loadaddr} ${preboot_sec_offs} 1 block */
		goto usage;

	return 0;
usage:
	printf("Usage: uboot_write\n");
	return 1;
}

static int do_preboot_update(cmd_tbl_t *cmdtp, int flag,int argc, char *argv[])
{
	int ret;
	char *arg = NULL;

	if (argc > 2)
		goto usage;

	if (argc == 2)
		arg = argv[1];

	ret = do_file_load("preboot_file",arg);
	if (ret)
		goto usage;

	ret = do_preboot_write();
	if (ret)
		goto usage;

	return 0;
usage:
	cmd_usage(cmdtp);
	return 1;
}


static int do_uboot_write(void)
{
	int uboot_sf_size;
	char *uboot_offset;
	char *uboot_file_size;
	char *uboot_load_address;
	char fixed_uboot_size[32];
	char cmdbuf[CONFIG_SYS_CBSIZE];

	uboot_offset = getenv("uboot_offs");
	uboot_file_size = getenv("filesize");
	uboot_load_address = getenv("loadaddr");

	/* Check environment variables */
	if (!uboot_offset || !uboot_file_size || !uboot_load_address) {
		printf("Environment variables are not defined:\n");
		printf("uboot_offs:%s\n",uboot_offset);
		printf("filesize:%s\n",uboot_file_size);
		printf("loadaddr:%s\n",uboot_load_address);
		goto usage;
	}

	/* Check file size */
	uboot_sf_size = simple_strtoul(uboot_file_size, NULL, 16);
	if (uboot_sf_size > UBOOT_SIZE) {
		printf("file size is larger then %x\n",UBOOT_SIZE);
		goto usage;
	}

	if (run_command ("sf probe 0", 0) < 0) /* sf probe 0 */
		goto usage;

	if (uboot_sf_size % CONFIG_ENV_SECT_SIZE)
		uboot_sf_size = ((uboot_sf_size / CONFIG_ENV_SECT_SIZE) + 1) * CONFIG_ENV_SECT_SIZE;

	sprintf(fixed_uboot_size,"%x",uboot_sf_size);
	strcpy(cmdbuf,"sf erase ");
	strcat(cmdbuf,uboot_offset);
	strcat(cmdbuf," ");
	strcat(cmdbuf,fixed_uboot_size);

	if (run_command (cmdbuf, 0) < 0) /* sf erase ${uboot_offs} ${uboot_sf_size} */
		goto usage;

	strcpy(cmdbuf,"sf write ");
	strcat(cmdbuf,uboot_load_address);
	strcat(cmdbuf," ");
	strcat(cmdbuf,uboot_offset);
	strcat(cmdbuf," ");
	strcat(cmdbuf,uboot_file_size);

	if (run_command (cmdbuf, 0) < 0) /* sf  write ${loadaddr} ${uboot_offs} ${filesize} */
		goto usage;

	return 0;
usage:
	printf("Usage: uboot_write\n");
	return 1;
}

static int do_uboot_update(cmd_tbl_t *cmdtp, int flag,int argc, char *argv[])
{
	int ret;
	char *arg = NULL;

	if (argc > 2)
		goto usage;

	if (argc == 2)
		arg = argv[1];

	ret = do_file_load("uboot_file",arg);
	if (ret)
		goto usage;

	ret = do_uboot_write();
	if (ret)
		goto usage;

	return 0;
usage:
	cmd_usage(cmdtp);
	return 1;
}


static int do_kernel_write(void)
{
	int kernel_sf_size;
	char *kernel_offset;
	char *kernel_file_size;
	char *kernel_load_address;
	char fixed_kernel_size[32];
	char cmdbuf[CONFIG_SYS_CBSIZE];

	kernel_offset = getenv("krn_offs");
	kernel_file_size = getenv("filesize");
	kernel_load_address = getenv("loadaddr");

	/* Check environment variables */
	if (!kernel_offset || !kernel_file_size || !kernel_load_address) {
		printf("Environment variables are not defined:\n");
		printf("krn_offs:%s\n",kernel_offset);
		printf("filesize:%s\n",kernel_file_size);
		printf("loadaddr:%s\n",kernel_load_address);
		goto usage;
	}

	/* Check file size */
	kernel_sf_size = simple_strtoul(kernel_file_size, NULL, 16);
	if (kernel_sf_size > KERNEL_SIZE) {
		printf("file size is larger then %x\n",KERNEL_SIZE);
		goto usage;
	}

	if (run_command ("sf probe 0", 0) < 0) /* sf probe 0 */
		goto usage;

	/* Round kernel_sf_size up */
	if (kernel_sf_size % CONFIG_ENV_SECT_SIZE)
		kernel_sf_size = ((kernel_sf_size / CONFIG_ENV_SECT_SIZE) + 1) * CONFIG_ENV_SECT_SIZE;

	sprintf(fixed_kernel_size,"%x",kernel_sf_size);
	strcpy(cmdbuf,"sf erase ");
	strcat(cmdbuf,kernel_offset);
	strcat(cmdbuf," ");
	strcat(cmdbuf,fixed_kernel_size);

	if (run_command (cmdbuf, 0) < 0) /* sf erase ${krn_offs} ${kernel_sf_size} */
		goto usage;

	strcpy(cmdbuf,"sf write ");
	strcat(cmdbuf,kernel_load_address);
	strcat(cmdbuf," ");
	strcat(cmdbuf,kernel_offset);
	strcat(cmdbuf," ");
	strcat(cmdbuf,kernel_file_size);

	if (run_command (cmdbuf, 0) < 0) /* sf  write ${loadaddr} ${krn_offs} ${filesize} */
		goto usage;

	return 0;
usage:
	printf("Usage: krn_write\n");
	return 1;
}

static int do_kernel_update(cmd_tbl_t *cmdtp, int flag,int argc, char *argv[])
{
	int ret;
	char *arg = NULL;

	if (argc > 2)
		goto usage;

	if (argc == 2)
		arg = argv[1];

	ret = do_file_load("krn_file",arg);
	if (ret)
		goto usage;

	ret = do_kernel_write();
	if (ret)
		goto usage;

	return 0;
usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	resetenv, 2, 0, do_resetenv,
	"reset environment to default settings",
	"Usage: resetenv [full]"
);

U_BOOT_CMD(
	upkrn_sz, 1, 0, do_upkrn_sz,
	"update env krn_size from uImage header ",
	"Usage: upkrn_sz"
);

U_BOOT_CMD(
	preboup, 2, 0, do_preboot_update,
	"Write preboot to SPI flash in ${preboot_offs}",
	"Usage: preboot_update [serial]"
);

U_BOOT_CMD(
	ubootup, 2, 0, do_uboot_update,
	"Write uboot to SPI flash in ${uboot_offs}",
	"Usage: uboot_update [serial]"
);

U_BOOT_CMD(
	krnup, 2, 0, do_kernel_update,
	"Write kernel to SPI flash in ${krn_offs}",
	"Usage: krn_update [serial]"
);

U_BOOT_CMD(
	mt,	4,	1,	do_mem_test,
	"basic RAM test",
	"test number:\n"
	"1-BWL, 2-roll01, 3-overlap, 4-fill, 5-random\n"
	"6-timestamp, 7-cache performance, 8-dcache e/d\n"
	"9-print AuxReg, a-active cores"

);
