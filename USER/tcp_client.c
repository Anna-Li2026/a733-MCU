#include "tcp_client.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "bsp_burnin_board.h"
#include "socket.h"

/* start burning function */
static int parse_start_burning_command(char* cmd, StartBurningParams_t* params);
static void send_start_burning_ack1(uint8_t sn, uint16_t timeout);
static void send_start_burning_ack2(uint8_t sn);
static void print_start_burning_params(StartBurningParams_t* params);
static void process_start_burning_boards(StartBurningParams_t* params);
/* read result function */
static int parse_read_result_command(char* cmd, ReadResultParams_t* params);
static void send_read_result_ack1(uint8_t sn, uint16_t timeout);
static void send_read_result_ack2(uint8_t sn, int mb, int site, TestResult_t* result);
static int process_read_result_boards(ReadResultParams_t* params, uint8_t sn, TestResult_t* result);
static void print_read_result_params(ReadResultParams_t* params);
/* caculate chaecksum function */
static uint32_t calc_checksum(const uint8_t* data,uint32_t size);
static uint32_t calc_checksum_result(int mb, int site, TestResult_t* result);

/**
 * @brief Process received command from host
 * @param sn Socket number
 * @param cmd Command string
 * @param len Command length
 */
int process_received_command(uint8_t sn, uint8_t* cmd, int len)
{
    // check the params
	if (cmd == NULL || *cmd == NULL) {
        printf("[ERROR]: Command pointer is NULL\r\n");
        return -1;
    }
    if (len == NULL) {
        printf("[ERROR]: Length pointer is NULL\r\n");
        return -1;
    }

	// Remove trailing newline characters if present
    if (len > 0 && cmd[len - 1] == '\n') {
        cmd[len - 1] = '\0';
        len--;
    }
    if (len > 0 && cmd[len - 1] == '\r') {
        cmd[len - 1] = '\0';
        len--;
    }

	// Check if command is empty after trimming
    if (len == 0) {
        printf("[ERROR]: Command is empty after trimming\r\n");
        return -1;
    }

    // Command routing
	if(strstr((char*)cmd, CMD_READ_RESULT_PASS) != NULL){
		printf("[LOG]: Read Result success !\r\n");
	}
    else if (strstr((char*)cmd, CMD_START_BURNING) != NULL) {
        handle_start_burning(sn, (char*)cmd);
    } 
    else if (strstr((char*)cmd, CMD_READ_RESULT) != NULL) {
        handle_read_result(sn,(char*) cmd);
    } 
    else {
        printf("[ERROR]: Unknown command: %s\r\n", cmd);
    }
	return 0;
}
/**
 * @brief Initialize TCP client network configuration
 * @param local_ip Pointer to 4-byte array containing local IP address
 * @param subnet Pointer to 4-byte array containing subnet mask
 * @param gateway Pointer to 4-byte array containing gateway address
 */
void TCP_Client_Init(uint8_t local_ip[4], uint8_t subnet[4], uint8_t gateway[4])
{
    // Set and verify local IP address
    setSIPR(local_ip);
    getSIPR(local_ip);
    printf("[LOG]:W5500 IP address is: %d.%d.%d.%d\r\n", local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
    
    // Set and verify subnet mask
    setSUBR(subnet);
    getSUBR(subnet);
    printf("[LOG]:W5500 IP mask is: %d.%d.%d.%d\r\n", subnet[0], subnet[1], subnet[2], subnet[3]);
    
    // Set and verify gateway address
    setGAR(gateway);
    getGAR(gateway);
    printf("[LOG]:W5500 Gateway is: %d.%d.%d.%d\r\n", gateway[0], gateway[1], gateway[2], gateway[3]);
}
/**
 * @brief Handle start burning command from host
 * @param sn Socket number
 * @param cmd Command string received from host
 */
void handle_start_burning(uint8_t sn, char* cmd)
{
    StartBurningParams_t params = {0};

    // Parse command string and extract parameters
    if(parse_start_burning_command(cmd, &params) != 0) {
        printf("[ERROR]:Parse start burning command failed!\r\n");
        return;
    }

    // Send response 1: Timeout period is 120 seconds
    send_start_burning_ack1(sn, 120);

    // Send response 2: Command execution success
    send_start_burning_ack2(sn);

    // Start up the motherboard and subboard, format EEPROM, write test command
    process_start_burning_boards(&params);

    // Print parsed parameters for debugging
    print_start_burning_params(&params);
}

/**
 * @brief Handle read result command from host
 * @param sn Socket number
 * @param cmd Command string received from host
 */
void handle_read_result(uint8_t sn, char* cmd)
{
    ReadResultParams_t params = {0};
    TestResult_t result = {0};

    // Parse command string and extract parameters
    if(parse_read_result_command(cmd, &params) != 0) {
        printf("[ERROR]:Parse read result command failed!\r\n");
        return;
    }

    // Send response 1: Timeout period is 120 seconds
    send_read_result_ack1(sn, 120);

    // Read test results from motherboard and subboard EEPROM
    process_read_result_boards(&params, sn, &result);

    // Print parsed parameters for debugging
    print_read_result_params(&params);
}

/**
* @brief Parse the start test command
* @param cmd Command string
* @param params Pointer to store the parsing result
* @return 0: Success, -1: Failure
*/
static int parse_start_burning_command(char* cmd, StartBurningParams_t* params)
{
    char* token;
    char* saveptr;
    int param_count;

    memset(params, 0, sizeof(StartBurningParams_t));

    // skip the command "@SStartBurning"
    token = strtok_r(cmd, "#", &saveptr);
    if(token == NULL)
	{printf("[ERROR]:start burning param is null\r\n");return -1;}

    // the number of para
    token = strtok_r(NULL, "#", &saveptr);
    if(token == NULL) 
	{printf("[ERROR]:start burning param->param_count is null\r\n");return -1;}
    param_count = atoi(token);

    //check the count para
    if(param_count <= 0) 
	{printf("[ERROR]:start burning param->param_count is %d\r\n",param_count);return -1;}

    // para1 mb index(0-3,0x0f-all)
    token = strtok_r(NULL, "#", &saveptr);
	if(strstr(token,"F")){
		params->mb_index = 0xf;
	}else{
		if(token) params->mb_index = (uint8_t)atoi(token);
		if(params->mb_index >= MAX_MB_COUNT && params->mb_index != ALL_MB_SELECT)
		{printf("[ERROR]:start burning param->mb_index is %d\r\n",params->mb_index);return -1;}
	}

    // para2 bib index(0-74,0xff-all)
    token = strtok_r(NULL, "#", &saveptr);
	if(strstr(token,"FF")){
		params->site_index = 0xff;
	}else{
		if(token) params->site_index = (uint8_t)atoi(token);
		if(params->site_index >= MAX_SITE_COUNT && params->site_index != ALL_SITE_SELECT)
		{printf("[ERROR]:start burning param->site_index is %d\r\n",params->site_index);return -1;}
	}
//    // para3 temperature(10-90)
//    token = strtok_r(NULL, "#", &saveptr);
//    if(token) params->temperature = (uint8_t)atoi(token);
//	if(params->temperature < MIN_TEMPERATURE || params->temperature > MAX_TEMPERATURE)
//	{printf("[ERROR]:start burning param->temperature is %d\r\n",params->temperature);return -1;}

//    // para4 time -- hour(0-60)
//    token = strtok_r(NULL, "#", &saveptr);
//    if(token) params->duration_hour = (uint8_t)atoi(token);
//	if(params->duration_hour > MAX_DURATION_HOUR)
//	{printf("[ERROR]:start burning param->duration_hour is %d\r\n",params->duration_hour);return -1;}

//    // para5 time -- min(0-60)
//    token = strtok_r(NULL, "#", &saveptr);
//    if(token) params->duration_min = (uint8_t)atoi(token);
//	if(params->duration_min > MAX_DURATION_MIN)
//	{printf("[ERROR]:start burning param->duration_min is %d\r\n",params->duration_min);return -1;}
//	
//    // para6 test software(0-0x0f)
//    token = strtok_r(NULL, "#", &saveptr);
//    if(token) params->test_software = (uint8_t)atoi(token);
//	if(params->test_software > MAX_TEST_SOFTWARE)
//	{printf("[ERROR]:start burning param->test_software is %d\r\n",params->test_software);return -1;}

//    // para7 test project(0-0xFFFFFFFF)
//    token = strtok_r(NULL, "#", &saveptr);
//    if(token) params->test_project = (uint32_t)strtoul(token, NULL, 16);

    // para8 test checksum
    token = strtok_r(NULL, "#", &saveptr);
    if(token) params->checksum = (uint32_t)strtoul(token, NULL, 16);

    return 0;
}

/**
* @brief Send response 1 (timeout duration)
* @param sn Socket number
* @param timeout Timeout duration (seconds)
*/
static void send_start_burning_ack1(uint8_t sn, uint16_t timeout)
{
    char response[64];
    char timeout_str[16];
    uint32_t checksum;

    sprintf(timeout_str, "%d", timeout);
    checksum = 120;

    snprintf(response, sizeof(response), "@SStartBurning#2#%d#%d#E@",
             timeout, checksum);
    send(sn, (uint8_t*)response, strlen(response));

    printf("[LOG]:Send ACK1: timeout=%d seconds\r\n", timeout);
}

/**
* @brief Send response 2 (failure)
* @param sn Socket number
*/
static void send_start_burning_ack2(uint8_t sn)
{
    char response[] = "@SStartBurningPass#E@";
    send(sn, (uint8_t*)response, strlen(response));
    printf("[LOG]:Send ACK2: PASS\r\n");
}

/**
* @brief Processing of the board card that executes the test command
* @param params Pointer to the test parameter structure
* @return 0: Success, -1: Failure
*/
static void process_start_burning_boards(StartBurningParams_t* params)
{
    uint8_t mb_start = 0, mb_end = 0;
    uint8_t site_start = 0, site_end = 0;
	uint16_t success_count = 0;
    uint16_t total_count = 0;
	uint8_t start_buf[4]={0};
	uint8_t check_buf[4]={0};

	// check mb index
    if(params->mb_index == 0xF) {
        mb_start = 0;
        mb_end = MAX_MB_COUNT;
    } else {
		if(params->mb_index >= MAX_MB_COUNT){
			printf("[ERROR]:start burning param->mb_index is %d\r\n",params->mb_index); return;
		}
        mb_start = params->mb_index;
        mb_end = params->mb_index + 1;
    }
	// send to mb-bib eeprom
    for(int mb = mb_start; mb < mb_end; mb++) {
		// check bib index
        if(params->site_index == 0xFF) {
            site_start = 0;
            site_end = MAX_SITE_COUNT;
        } else {
			if(params->site_index >= MAX_SITE_COUNT){
				printf("[ERROR]:start burning param->site_index is %d\r\n",params->site_index); return;
			}
            site_start = params->site_index;
            site_end = params->site_index + 1;
        }

        for(int site = site_start; site < site_end; site++) {
			//check the bib
//			if(!check_online(site, 0)) {
//				printf("[ERROR]:BIB%d is offline, skip!\r\n", site);
//				continue;
//			}
            total_count++;

//			start_buf[0] = params->temperature;
//			start_buf[1] = params->duration_hour;
//			start_buf[2] = params->duration_min;
//			start_buf[3] = params->test_software;
//			*(uint32_t*)&start_buf[4] = params->test_project;

            // write test command from EEPROM
            if(mb_eep_write(mb, site, EEPROM_BURNING_START, EEPROM_BURNING_LEN, start_buf) != 1){
				printf("[ERROR]:start burning mb %d bib %d write eeprom fail\r\n",mb,site);continue;
			}
			mb_eep_write(mb, site, EEPROM_BURNING_START2, EEPROM_BURNING_LEN, start_buf);
			
			mb_eep_read(mb, site, EEPROM_BURNING_START, EEPROM_BURNING_LEN, check_buf);
			if (memcmp(start_buf, check_buf, 4) != 0){
				printf("[ERROR]:start burning mb %d bib %d eeprom check fail\r\n",mb,site);continue;
			}
			success_count++;
        }
    }
    printf("[LOG]:Start burning completed! Success: %d/%d\r\n", success_count, total_count);
}

/**
* @brief Prints the command parameters for the start test
* @param params Pointer to the test parameter structure
*/
static void print_start_burning_params(StartBurningParams_t* params)
{
    printf("\r\n=== Start Burning Command ===\r\n");
    printf("MB Index: 0x%02X (%s)\r\n", params->mb_index,
           params->mb_index == 0xF ? "ALL" : "Single");
    printf("Site Index: 0x%02X (%s)\r\n", params->site_index,
           params->site_index == 0xFF ? "ALL" : "Single");
    printf("Temperature: %dC\r\n", params->temperature);
    printf("Duration: %dH %dm\r\n", params->duration_hour, params->duration_min);
    printf("Test Software: %d\r\n", params->test_software);
    printf("Test Project: 0x%08X\r\n", params->test_project);
    printf("Checksum: %d\r\n", params->checksum);
    printf("=============================\r\n");
}

/**
* @brief Parses the read result command
* @param cmd Command string
* @param params Pointer to store the parsing result
* @return 0: Success, -1: Failure
*/
static int parse_read_result_command(char* cmd, ReadResultParams_t* params)
{
    char* token;
    char* saveptr;
    int param_count;

    memset(params, 0, sizeof(ReadResultParams_t));

    // Skip the command "@SReadResult"
    token = strtok_r(cmd, "#", &saveptr);
    if(token == NULL) 
	{printf("[ERROR]:read result param is null\r\n");return -1;}

    // Obtain the number of parameters
    token = strtok_r(NULL, "#", &saveptr);
    if(token == NULL) 
	{printf("[ERROR]:read result param->param_count is null\r\n");return -1;}
    param_count = atoi(token);

    if(param_count != 3)
	{printf("[ERROR]:read result param->param_count is %d\r\n",param_count);return -1;}

    // para1 mb index(0-3,"f"-all)
    token = strtok_r(NULL, "#", &saveptr);
	if(strstr(token,"F")){
		params->mb_index = 0xf;
	}else{
		if(token) params->mb_index = (uint8_t)atoi(token);
		if(params->mb_index >= MAX_MB_COUNT && params->mb_index != ALL_MB_SELECT)
		{printf("[ERROR]:read result param->mb_index is %d\r\n",params->mb_index);return -1;}
	}
    // para2 bib index(0-74,"ff"-all)
    token = strtok_r(NULL, "#", &saveptr);
	if(strstr(token,"FF")){
		params->site_index = 0xff;
	}else{
		if(token) params->site_index = (uint8_t)atoi(token);
		if(params->site_index >= MAX_SITE_COUNT && params->site_index != ALL_SITE_SELECT)
		{printf("[ERROR]:read result param->site_index is %d\r\n",params->site_index);return -1;}
	}
    
    // para3 checksum
    token = strtok_r(NULL, "#", &saveptr);
    if(token) params->checksum = (uint32_t)strtoul(token, NULL, 16);

    return 0;
}

/**
* @brief Send read result response 1 (timeout duration)
* @param sn Socket number
* @param timeout Timeout duration (seconds) */
static void send_read_result_ack1(uint8_t sn, uint16_t timeout)
{
    uint8_t response[64];
    uint8_t timeout_str[16];
    uint32_t checksum;

    sprintf((char*)timeout_str, "%d", timeout);
    checksum = 120;

    snprintf((char*)response, sizeof(response), "@SReadResult#2#%d#%d#E@",
             timeout, checksum);
    send(sn, response, strlen((char*)response));

    printf("[LOG]:Send ReadResult ACK1: timeout=%d seconds\r\n", timeout);
}


/**
 * @brief Send read result acknowledgment 2 (response with test results)
 * @param sn Socket number
 * @param mb Motherboard number (0-3)
 * @param site Site/subboard number (0-75)
 * @param result Pointer to test result structure
 */
static void send_read_result_ack2(uint8_t sn, int mb, int site, TestResult_t* result)
{
    uint8_t response[120] = {0};
    uint32_t checksum = 0;
    uint8_t response_body[100] = {0};
    int len;

	// Calculate checksum (from first parameter "#3" to last parameter)
    checksum = calc_checksum_result(mb, site, result);
	
    // Build response body (excluding checksum and end marker)
	if(site == 0xff){
		for(int i = 0;i<MAX_SITE_COUNT;i++){
			response_body[i] = result->test_result_all_site[i];
		}
		len = snprintf((char*)response, sizeof(response),
			"@SReadResultPass#3#%d#%s#%d#E@",
			mb,                          				// Param1: Motherboard number
			response_body,
			checksum);
	}else{
		len = snprintf((char*)response, sizeof(response),
			"@SReadResultPass#4#%d#%d#%c#%d#E@",
			mb,                           		// Param1: Motherboard number
			site,                         		// Param2: Site/subboard number
			result->test_result_single_site,	// Param3: Test result ('P'=PASS)
			checksum);
	}

    // Assemble complete response
	send(sn, response, strlen((char*)response));
	if(site == 0xff){
		for(int i = 0;i<75;i++){
			printf("[LOG]:Send ReadResult ACK2: MB=%d, Site=%d, Result=%08X, Checksum=%d\r\n",
				mb, i, result->test_result_all_site[i], checksum);
		}
	}else{
		printf("[LOG]:Send ReadResult ACK2: MB=%d, Site=%d, Result=%08X, Checksum=%d\r\n",
				mb, site, result->test_result_single_site, checksum);
	}
}

/**
 * @brief Process read result operation for all specified boards
 * @param params Pointer to read result parameters structure
 * @param sn Socket number for TCP communication
 * @param result Pointer to test result structure for storing data
 * @return 0 on success (at least one result read), -1 on failure (no results read)
 */
static int process_read_result_boards(ReadResultParams_t* params, uint8_t sn, TestResult_t* result)
{
    uint8_t mb_start, mb_end;
    int result_count = 0;
    uint8_t result_buf[2];

    // Determine motherboard range based on parameters
    // 0xF means all motherboards, otherwise single board
    if(params->mb_index == 0xF) {
        mb_start = 0;
        mb_end = MAX_MB_COUNT;
    } else {
		if(params->mb_index >= MAX_MB_COUNT){
			printf("[ERROR]:read result param->mb_index is %d\r\n",params->mb_index); return -1;
		}
        mb_start = params->mb_index;
        mb_end = params->mb_index + 1;
    }

    // Iterate through all specified motherboards
    for(int mb = mb_start; mb < mb_end; mb++) {
        // Determine site/subboard range based on parameters
        // 0xFF means all sites, otherwise single site
        if(params->site_index == 0xFF) {
			// Iterate through all specified sites/subboards
			for(int site = 0; site < MAX_SITE_COUNT; site++) {
				// Check if the subboard is online/accessible
				if(!check_online(site, 0)) {
					printf("[ERROR]:BIB%d is offline, skip!\r\n", site);
					continue;
				}
				// Read test results from EEPROM
				if(mb_eep_read(mb, site, EEPROM_RESULT_START, EEPROM_RESULT_LEN, result_buf) == 1) {
					// Parse EEPROM data into result structure
					result->test_result_all_site[site] = result_buf[0];
					if(result->test_result_all_site[site] == 'P' || result->test_result_all_site[site] == 'F'){
						printf("[LOG]:test result is valid:mb %d, site %d, test result is %x\r\n",mb, site, result->test_result_all_site[site]); 
					}else{
						printf("[WARN]:test result is valid:mb %d, site %d, test result is %x\r\n",mb, site, result->test_result_all_site[site]); 
						result->test_result_all_site[site] = 'N';
					}
					result_count++;
				}
			}
        } else {
			if(params->site_index >= MAX_SITE_COUNT){
				printf("[ERROR]:read result param->site_index is %d\r\n",params->mb_index); return -1;
			}
			if(mb_eep_read(mb, params->site_index, EEPROM_RESULT_START, EEPROM_RESULT_LEN, result_buf) == 1) {
				result->test_result_single_site = result_buf[0];
				if(result->test_result_single_site == 'P' || result->test_result_single_site == 'F'){
					printf("[LOG]:test result is invalid:mb %d, site %d, test result is %x\r\n",params->mb_index, params->site_index, result->test_result_single_site); 
				}else{
					printf("[WARN]:test result is invalid:mb %d, site %d, test result is %x\r\n",params->mb_index, params->site_index, result->test_result_single_site); 
					result->test_result_single_site = 'N';
				}
				result_count++;
			}
        }
		// Send result back to host via TCP
		send_read_result_ack2(sn, mb, params->site_index, result);
    }

    printf("[LOG]:Read result completed! Total results: %d\r\n", result_count);
    return (result_count > 0) ? 0 : -1;
}

/**
* @brief Print the read result parameters
* @param params Pointer to the structure of read result parameters
*/
static void print_read_result_params(ReadResultParams_t* params)
{
    printf("\r\n=== Read Result Command ===\r\n");
    printf("MB Index: 0x%02X (%s)\r\n", params->mb_index,
           params->mb_index == 0xF ? "ALL" : "Single");
    printf("Site Index: 0x%02X (%s)\r\n", params->site_index,
           params->site_index == 0xFF ? "ALL" : "Single");
    printf("Checksum: %d\r\n", params->checksum);
    printf("=============================\r\n");
}

static uint32_t calc_checksum(const uint8_t* data,uint32_t size)
{
	int i;
	uint32_t sum = 0;
	uint8_t left_over[4] = {0};
	while(size>3){
		sum+=*((uint32_t*)data);
		data+=sizeof(uint32_t);
		size-=sizeof(uint32_t);
	}
	for(i=0;i<size;++i){
		left_over[i]= *((uint8_t*)data++);
	}

	sum+= *((uint32_t*)left_over);

	return ~sum;
}

static uint32_t calc_checksum_result(int mb, int site, TestResult_t* result)
{
	uint8_t cnt_p = 0;
	uint8_t cnt_f = 0;
	uint8_t cnt_n = 0;
	
	if(site == 0xff){
		for(int i = 0;i<75;i++){
			if(result->test_result_all_site[i] == 'P'){
				cnt_p++;
			}else if(result->test_result_all_site[i] == 'F'){
				cnt_f++;
			}else if(result->test_result_all_site[i] == 'N'){
				cnt_n++;
			}
		}
	}else{
		if(result->test_result_single_site == 'P'){
			cnt_p++;
		}else if(result->test_result_single_site == 'F'){
			cnt_f++;
		}else if(result->test_result_single_site == 'N'){
			cnt_n++;
		}
	}
	return mb + cnt_p + cnt_f*2 + cnt_n * 3;
}
