#ifndef OPENGYM_PARA_H
#define OPENGYM_PARA_H

#define host_port 8000
#define target_port 9000

#define REQUESTPORT 8000
#define RESULTPORT 	9000

#define INITRATE 350000
#define INITDELAY 5500
#define FILESIZE   100000000 //100MB
#define TARGETSIZE 300000000 //300MB

#define NUMOFTIER 3
static uint16_t TIERS[NUMOFTIER] = {1,1,1};

#define ZIPFALPHA 0.55

#define TARGETS 3
#define DATAS 8
#define USERS 24

#define OR_PAGESIZE 8192
#define REQUESTSIZE 
#define RESPONSESIZE 120
#define PAGESIZE 120
#define SPEEDUP OR_PAGESIZE/PAGESIZE
// If you modify SPEEDUP, must dividu PAGESIZE(4KB) by SPEEDUP
// And You must modify MTU to PAGESIZE

#define SERVICETIMEAVG 30000000
#define SERVICETIMESTD 10000000
#define MAXSERVICETIME 55000000
#define MINSERVICETIME  5000000


#define OBS_INTERVAL 1000
#define PC_INTERVAL 5000

#define SMOOTHING 4

#define QUEUE_DEPTH 32
#define MAX_TARGET_QUEUE 1024
#endif
