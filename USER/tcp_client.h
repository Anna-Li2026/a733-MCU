#ifndef _TCP_CLIENT_H
#define _TCP_CLIENT_H

#include <stdint.h>

//match the command from TCP server
#define CMD_START_BURNING   		"@SStartBurning"
#define CMD_READ_RESULT     		"@SReadResult"
#define CMD_READ_RESULT_PASS     	"@SReadResultPass"

//StartBurningParams_t.test_project : type of software test
#define TEST_STRESSNG           0
#define TEST_STRESSAPPTEST      1
#define TEST_APK                2
#define TEST_MEMTESTER          3
#define TEST_STRESS_APP_MEM     4

//max of mb and bib
#define MAX_MB_COUNT       		4
#define MAX_SITE_COUNT      	75
#define ALL_MB_SELECT       	0x0f
#define ALL_SITE_SELECT      	0xff

//max of test software
#define MAX_TEST_SOFTWARE		0x0f

//max of test hour and minute
#define MAX_DURATION_HOUR		60
#define MAX_DURATION_MIN		60

//min and max of test temperature
#define MIN_TEMPERATURE			10
#define MAX_TEMPERATURE			90

//eeprom addr and len to start burning
#define EEPROM_BURNING_START    0x00
#define EEPROM_BURNING_START2   0x10
#define EEPROM_BURNING_LEN     	4

//eeprom addr and len to read result
#define EEPROM_RESULT_START     0x00
#define EEPROM_RESULT_LEN     	1

//start burning packege struct
typedef struct {
    uint8_t mb_index;       		// 0-3 mother board index
    uint8_t site_index;     		// 0-74 site index
    uint8_t temperature;    		// text tempeature
    uint8_t duration_hour;  		// 0-60 hour
    uint8_t duration_min;   		// 0-60 min
    uint8_t test_software;  		// type of software test
    uint32_t test_project;  		// test project
    uint32_t checksum;      		// checksum
} StartBurningParams_t;

//read result packege struct
typedef struct {
    uint8_t mb_index;       		// 0-3 mother board index
    uint8_t site_index;     		// 0-74 site indext
    uint32_t checksum;      		// checksum
} ReadResultParams_t;

//test result struct
typedef struct {
//    uint8_t boot_flag;              // Boot status: 0=offline/not booted, 1=successfully booted into system (Byte 31)
    uint32_t test_result_all_site[75];           // Test result: 'P'=0X50=PASS, 'F'=0X46=FAIL (Byte 32-35)
    uint32_t test_result_single_site;           // Test result: 'P'=0X50=PASS, 'F'=0X46=FAIL (Byte 32-35)
//    uint8_t test_software;          // Test software type: 0=stressng, 1=stressapptest, 2=APK, 3=memtester, 4=stressapptest+memtester (Byte 36)
//    uint8_t fail_item;              // Failed test item number, range 0-128 (Byte 37)
//    uint32_t fail_addr1;            // Fail address high 32-bit (Byte 38-41)
//    uint32_t fail_addr2;            // Fail address low 32-bit (Byte 42-45)
//    uint32_t fail_data1;            // Fail data high 32-bit (Byte 46-49)
//    uint32_t fail_data2;            // Fail data low 32-bit (Byte 50-53)
//    uint8_t duration_hour;          // Actual test duration in hours, range 0-60 (Byte 54)
//    uint8_t duration_min;           // Actual test duration in minutes, range 0-60 (Byte 55)
} TestResult_t;

void TCP_Client_Init(uint8_t local_ip[4], uint8_t subnet[4], uint8_t gateway[4]);
int process_received_command(uint8_t sn, uint8_t* cmd, int len);
void handle_start_burning(uint8_t sn, char* cmd);
void handle_read_result(uint8_t sn, char* cmd);

#endif
