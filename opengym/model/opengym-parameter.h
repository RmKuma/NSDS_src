#ifndef OPENGYM_PARA_H
#define OPENGYM_PARA_H

#define host_port 8000
#define target_port 9000

#define REQUESTPORT 8000
#define RESULTPORT 	9000

#define INITRATE 350000
#define INITDELAY 5500
#define FILESIZE   100000 //KB

#define NUMOFTIER 3
static uint16_t TIERS[NUMOFTIER] = {1,1,1};

#define ZIPFALPHA 0.55

#define TARGETS 3
#define DATAS 10 
#define USERS 40

#define TARGETSIZE FILESIZE*DATAS //300MB

#define OR_PAGESIZE 4096
#define RESPONSESIZE 120
#define PAGESIZE 120
#define SPEEDUP OR_PAGESIZE/PAGESIZE
// If you modify SPEEDUP, must dividu PAGESIZE(4KB) by SPEEDUP
// And You must modify MTU to PAGESIZE

#define SERVICETIMEAVG 30000000
#define SERVICETIMESTD 10000000
#define MAXSERVICETIME 55000000
#define MINSERVICETIME  5000000
#define MIGRATIONTIME SERVICETIMEAVG/3

#define OBS_INTERVAL 1000
#define PC_INTERVAL 5000

#define SMOOTHING 4

#define QUEUE_DEPTH 32
#define MAX_TARGET_QUEUE 1024

#define DELAY_UP 1
#define DELAY_A_BASE  9
#define DELAY_A_MULTI 9
#define DELAY_B_BASE  800
#define DELAY_B_MULTI 400

#endif
