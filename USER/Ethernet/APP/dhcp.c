/**
* @file dhcp.c
*
* Copyright (c) Artmem 2022. All Rights Reserved.
*
* @brief
*
* @details DHCP main program body
*
* @date 2022/9/29
*
* @author Jeff.he
*
* @bug
*
* Revisions: v1.0
*
*/

#include <string.h>
#include "trace.h"
#include "socket.h"
#include "dhcp.h"

/*
 * @brief
 * @details If you want to display debug & procssing message, Define _DHCP_DEBUG_
 * @note    If defined, it dependens on <stdio.h>
 */
#ifdef _DHCP_DEBUG_
#define DHCP_DBG(fmt, ...)      DBG(fmt, ##__VA_ARGS__)
#define DHCP_WRN(fmt, ...)      WRN(fmt, ##__VA_ARGS__)
#define DHCP_ERR(fmt, ...)      ERR(fmt, ##__VA_ARGS__)
#define DHCP_DUMP(buf,len)      DUMP("DHCP", buf, len)
#else
#define DHCP_DBG(fmt, ...)
#define DHCP_WRN(fmt, ...)
#define DHCP_ERR(fmt, ...)
#define DHCP_DUMP(buf,len)
#endif

 /* DHCP state machine. */
#define STATE_DHCP_INIT          0        ///< Initialize
#define STATE_DHCP_DISCOVER      1        ///< send DISCOVER and wait OFFER
#define STATE_DHCP_REQUEST       2        ///< send REQEUST and wait ACK or NACK
#define STATE_DHCP_LEASED        3        ///< ReceiveD ACK and IP leased
#define STATE_DHCP_REREQUEST     4        ///< send REQUEST for maintaining leased IP
#define STATE_DHCP_RELEASE       5        ///< No use
#define STATE_DHCP_STOP          6        ///< Stop procssing DHCP

#define DHCP_FLAGSBROADCAST      0x8000   ///< The broadcast value of flags in @ref RIP_MSG 
#define DHCP_FLAGSUNICAST        0x0000   ///< The unicast   value of flags in @ref RIP_MSG

/* DHCP message OP code */
#define DHCP_BOOTREQUEST         1        ///< Request Message used in op of @ref RIP_MSG
#define DHCP_BOOTREPLY           2        ///< Reply Message used i op of @ref RIP_MSG

/* DHCP message type */
#define DHCP_DISCOVER            1        ///< DISCOVER message in OPT of @ref RIP_MSG
#define DHCP_OFFER               2        ///< OFFER message in OPT of @ref RIP_MSG
#define DHCP_REQUEST             3        ///< REQUEST message in OPT of @ref RIP_MSG
#define DHCP_DECLINE             4        ///< DECLINE message in OPT of @ref RIP_MSG
#define DHCP_ACK                 5        ///< ACK message in OPT of @ref RIP_MSG
#define DHCP_NAK                 6        ///< NACK message in OPT of @ref RIP_MSG
#define DHCP_RELEASE             7        ///< RELEASE message in OPT of @ref RIP_MSG. No use
#define DHCP_INFORM              8        ///< INFORM message in OPT of @ref RIP_MSG. No use

#define DHCP_HTYPE10MB           1        ///< Used in type of @ref RIP_MSG
#define DHCP_HTYPE100MB          2        ///< Used in type of @ref RIP_MSG

#define DHCP_HLENETHERNET        6        ///< Used in hlen of @ref RIP_MSG
#define DHCP_HOPS                0        ///< Used in hops of @ref RIP_MSG
#define DHCP_SECS                0        ///< Used in secs of @ref RIP_MSG

#define INFINITE_LEASETIME       0xffffffff    ///< Infinite lease time

#define OPT_SIZE                 312               /// Max OPT size of @ref RIP_MSG
#define RIP_MSG_SIZE             (236+OPT_SIZE)    /// Max size of @ref RIP_MSG

/*
 * @brief DHCP option and value (cf. RFC1533)
 */
enum
{
    padOption = 0,
    subnetMask = 1,
    timerOffset = 2,
    routersOnSubnet = 3,
    timeServer = 4,
    nameServer = 5,
    dns = 6,
    logServer = 7,
    cookieServer = 8,
    lprServer = 9,
    impressServer = 10,
    resourceLocationServer = 11,
    hostName = 12,
    bootFileSize = 13,
    meritDumpFile = 14,
    domainName = 15,
    swapServer = 16,
    rootPath = 17,
    extentionsPath = 18,
    IPforwarding = 19,
    nonLocalSourceRouting = 20,
    policyFilter = 21,
    maxDgramReasmSize = 22,
    defaultIPTTL = 23,
    pathMTUagingTimeout = 24,
    pathMTUplateauTable = 25,
    ifMTU = 26,
    allSubnetsLocal = 27,
    broadcastAddr = 28,
    performMaskDiscovery = 29,
    maskSupplier = 30,
    performRouterDiscovery = 31,
    routerSolicitationAddr = 32,
    staticRoute = 33,
    trailerEncapsulation = 34,
    arpCacheTimeout = 35,
    ethernetEncapsulation = 36,
    tcpDefaultTTL = 37,
    tcpKeepaliveInterval = 38,
    tcpKeepaliveGarbage = 39,
    nisDomainName = 40,
    nisServers = 41,
    ntpServers = 42,
    vendorSpecificInfo = 43,
    netBIOSnameServer = 44,
    netBIOSdgramDistServer = 45,
    netBIOSnodeType = 46,
    netBIOSscope = 47,
    xFontServer = 48,
    xDisplayManager = 49,
    dhcpRequestedIPaddr = 50,
    dhcpIPaddrLeaseTime = 51,
    dhcpOptionOverload = 52,
    dhcpMessageType = 53,
    dhcpServerIdentifier = 54,
    dhcpParamRequest = 55,
    dhcpMsg = 56,
    dhcpMaxMsgSize = 57,
    dhcpT1value = 58,
    dhcpT2value = 59,
    dhcpClassIdentifier = 60,
    dhcpClientIdentifier = 61,
    endOption = 255
};

/*
 * @brief DHCP message format
 */
typedef struct {
    uint8_t  op;            ///< @ref DHCP_BOOTREQUEST or @ref DHCP_BOOTREPLY
    uint8_t  htype;         ///< @ref DHCP_HTYPE10MB or @ref DHCP_HTYPE100MB
    uint8_t  hlen;          ///< @ref DHCP_HLENETHERNET
    uint8_t  hops;          ///< @ref DHCP_HOPS
    uint32_t xid;           ///< @ref dhcp.xid  This increase one every DHCP transaction.
    uint16_t secs;          ///< @ref DHCP_SECS
    uint16_t flags;         ///< @ref DHCP_FLAGSBROADCAST or @ref DHCP_FLAGSUNICAST
    uint8_t  ciaddr[4];     ///< @ref Request IP to DHCP sever
    uint8_t  yiaddr[4];     ///< @ref Offered IP from DHCP server
    uint8_t  siaddr[4];     ///< No use 
    uint8_t  giaddr[4];     ///< No use
    uint8_t  chaddr[16];    ///< DHCP client 6bytes MAC address. Others is filled to zero
    uint8_t  sname[64];     ///< No use
    uint8_t  file[128];     ///< No use
    uint8_t  OPT[0];         ///< Option variable
} RIP_MSG;


/* define struct dhcp */
typedef struct {
    uint8_t mac[6];                 // DHCP Client MAC address.
    uint8_t sip[4];                 // DHCP Server IP address
    uint8_t sn;                    // Socket number for DHCP    
    uint32_t xid;                 // Any number

    uint8_t old_ip[4];            // Previous IP address
    uint8_t allocated_ip[4];    // IP address from DHCP
    uint8_t allocated_gw[4];    // Gateway address from DHCP
    uint8_t allocated_sn[4];    // Subnet mask from DHCP
    uint8_t allocated_dns[4];   // DNS address from DHCP
    uint32_t lease_time;

    int8_t state;                // DHCP state
    int8_t retry_count;
    uint32_t tick_1s;            // unit 1 second
    uint32_t tick_next;

    active_t assign_ip;            //handler to be called when the IP address from DHCP server is first assigned
    active_t update_ip;            //handler to be called when the IP address from DHCP server is updated
    active_t conflict_ip;        //handler to be called when the IP address from DHCP server is conflict

}dhcp_t;

/* extern The default callback function */
void default_assign_ip(void);
void default_update_ip(void);
void default_conflict_ip(void);


/* default variable */
static RIP_MSG* pDHCPMSG;      // Buffer pointer for DHCP processing


static dhcp_t dhcp = {

    .xid = 0x12345678,

    .old_ip = {0,0,0,0},
    .allocated_ip = {0,0,0,0},
    .allocated_gw = {0,0,0,0},
    .allocated_sn = {0,0,0,0},
    .allocated_dns = {0,0,0,0},
    .lease_time = INFINITE_LEASETIME,

    .state = STATE_DHCP_INIT,
    .retry_count = 0,
    .tick_1s = 0,
    .tick_next = DHCP_WAIT_TIME,

    .assign_ip = default_assign_ip,
    .update_ip = default_update_ip,
    .conflict_ip = default_conflict_ip,
};


/* The default handler of ip assign first */
void default_assign_ip(void)
{
    setSIPR(dhcp.allocated_ip);
    setSUBR(dhcp.allocated_sn);
    setGAR(dhcp.allocated_gw);
}

/* The default handler of ip chaged */
void default_update_ip(void)
{
    /* WIZchip Software Reset */
    setMR(MR_RST);
    getMR(); // for delay
    default_assign_ip();
    setSHAR(dhcp.mac);
}

/* The default handler of ip chaged */
void default_conflict_ip(void)
{
    // WIZchip Software Reset
    setMR(MR_RST);
    getMR(); // for delay
    setSHAR(dhcp.mac);
}

/* register the call back func. */
void reg_dhcp_cbfunc(active_t assign_ip, active_t update_ip, active_t conflict_ip)
{
    dhcp.assign_ip = default_assign_ip;
    dhcp.update_ip = default_update_ip;
    dhcp.conflict_ip = default_conflict_ip;
    if (assign_ip)   dhcp.assign_ip = assign_ip;
    if (update_ip)   dhcp.update_ip = update_ip;
    if (conflict_ip) dhcp.conflict_ip = conflict_ip;
}


/* SEND DHCP DISCOVER */
static void send_DHCP_DISCOVER(void)
{
    //broadcast ip
    uint8_t ip[4] = { 255, 255, 255, 255 };

    memset(pDHCPMSG, 0, RIP_MSG_SIZE);

    //init message head
    pDHCPMSG->op = DHCP_BOOTREQUEST;
    pDHCPMSG->htype = DHCP_HTYPE10MB;
    pDHCPMSG->hlen = DHCP_HLENETHERNET;
    pDHCPMSG->hops = DHCP_HOPS;
    pDHCPMSG->xid = htonl(dhcp.xid);
    pDHCPMSG->secs = htons(DHCP_SECS);
    memcpy(pDHCPMSG->chaddr, dhcp.mac, 6);

    // Option Request Param
    // MAGIC_COOKIE
    *((uint32_t*)pDHCPMSG->OPT) = htonl(MAGIC_COOKIE);

    //dhcpMessageType option
    pDHCPMSG->OPT[4] = dhcpMessageType;
    pDHCPMSG->OPT[5] = 0x01;
    pDHCPMSG->OPT[6] = DHCP_DISCOVER;

    // Client identifier option
    pDHCPMSG->OPT[7] = dhcpClientIdentifier;
    pDHCPMSG->OPT[8] = 0x07;
    pDHCPMSG->OPT[9] = 0x01;
    pDHCPMSG->OPT[10] = dhcp.mac[0];
    pDHCPMSG->OPT[11] = dhcp.mac[1];
    pDHCPMSG->OPT[12] = dhcp.mac[2];
    pDHCPMSG->OPT[13] = dhcp.mac[3];
    pDHCPMSG->OPT[14] = dhcp.mac[4];
    pDHCPMSG->OPT[15] = dhcp.mac[5];

    // host name option
    pDHCPMSG->OPT[16] = hostName;
    pDHCPMSG->OPT[17] = 7;
    pDHCPMSG->OPT[18] = 'W';
    pDHCPMSG->OPT[19] = 'I';
    pDHCPMSG->OPT[20] = 'Z';
    pDHCPMSG->OPT[21] = dhcp.mac[3];
    pDHCPMSG->OPT[22] = dhcp.mac[4];
    pDHCPMSG->OPT[23] = dhcp.mac[5];
    pDHCPMSG->OPT[24] = '\0';

    //dhcpParamRequest option
    pDHCPMSG->OPT[25] = dhcpParamRequest;
    pDHCPMSG->OPT[26] = 0x06;
    pDHCPMSG->OPT[27] = subnetMask;
    pDHCPMSG->OPT[28] = routersOnSubnet;
    pDHCPMSG->OPT[29] = dns;
    pDHCPMSG->OPT[30] = domainName;
    pDHCPMSG->OPT[31] = dhcpT1value;
    pDHCPMSG->OPT[32] = dhcpT2value;

    //end option
    pDHCPMSG->OPT[33] = endOption;

    DHCP_DBG("> Send DHCP_DISCOVER\r\n");

    // send broadcasting packet
    sendto(dhcp.sn, (uint8_t*)pDHCPMSG, RIP_MSG_SIZE, ip, DHCP_SERVER_PORT);
}

/* SEND DHCP REQUEST */
static void send_DHCP_REQUEST(void)
{
    int i;
    uint8_t ip[4];

    memset(pDHCPMSG, 0, RIP_MSG_SIZE);

    //init message head
    pDHCPMSG->op = DHCP_BOOTREQUEST;
    pDHCPMSG->htype = DHCP_HTYPE10MB;
    pDHCPMSG->hlen = DHCP_HLENETHERNET;
    pDHCPMSG->hops = DHCP_HOPS;
    pDHCPMSG->xid = htonl(dhcp.xid);
    pDHCPMSG->secs = htons(DHCP_SECS);
    memcpy(pDHCPMSG->chaddr, dhcp.mac, 6);

    if (dhcp.state == STATE_DHCP_LEASED || dhcp.state == STATE_DHCP_REREQUEST)
    {
        pDHCPMSG->flags = htons(DHCP_FLAGSUNICAST);
        memcpy(pDHCPMSG->ciaddr, dhcp.allocated_ip, 4);
        memcpy(ip, dhcp.sip, 4);
    }
    else
    {    //ip broadcast
        memset(ip, 255, 4);
    }

    // Option Request Param
    // MAGIC_COOKIE
    *((uint32_t*)pDHCPMSG->OPT) = htonl(MAGIC_COOKIE);

    pDHCPMSG->OPT[4] = dhcpMessageType;
    pDHCPMSG->OPT[5] = 0x01;
    pDHCPMSG->OPT[6] = DHCP_REQUEST;

    pDHCPMSG->OPT[7] = dhcpClientIdentifier;
    pDHCPMSG->OPT[8] = 0x07;
    pDHCPMSG->OPT[9] = 0x01;
    pDHCPMSG->OPT[10] = dhcp.mac[0];
    pDHCPMSG->OPT[11] = dhcp.mac[1];
    pDHCPMSG->OPT[12] = dhcp.mac[2];
    pDHCPMSG->OPT[13] = dhcp.mac[3];
    pDHCPMSG->OPT[14] = dhcp.mac[4];
    pDHCPMSG->OPT[15] = dhcp.mac[5];

    i = 16;
    if (ip[3] == 255)
    {
        //dhcpRequestedIPaddr option
        pDHCPMSG->OPT[i + 0] = dhcpRequestedIPaddr;
        pDHCPMSG->OPT[i + 1] = 4;
        memcpy(pDHCPMSG->OPT + i + 2, dhcp.allocated_ip, 4);

        //dhcpServerIdentifier option
        pDHCPMSG->OPT[i + 6] = dhcpServerIdentifier;
        pDHCPMSG->OPT[i + 7] = 4;
        memcpy(pDHCPMSG->OPT + i + 8, dhcp.sip, 4);

        i += (2 + 4 + 2 + 4);
    }

    // host name option
    pDHCPMSG->OPT[i + 0] = hostName;
    pDHCPMSG->OPT[i + 1] = 7;
    pDHCPMSG->OPT[i + 2] = 'W';
    pDHCPMSG->OPT[i + 3] = 'I';
    pDHCPMSG->OPT[i + 4] = 'Z';
    pDHCPMSG->OPT[i + 5] = dhcp.mac[3];
    pDHCPMSG->OPT[i + 6] = dhcp.mac[4];
    pDHCPMSG->OPT[i + 7] = dhcp.mac[5];
    pDHCPMSG->OPT[i + 8] = '\0';

    //dhcpParamRequest option
    pDHCPMSG->OPT[i + 9] = dhcpParamRequest;
    pDHCPMSG->OPT[i + 10] = 0x08;
    pDHCPMSG->OPT[i + 11] = subnetMask;
    pDHCPMSG->OPT[i + 12] = routersOnSubnet;
    pDHCPMSG->OPT[i + 13] = dns;
    pDHCPMSG->OPT[i + 14] = domainName;
    pDHCPMSG->OPT[i + 15] = dhcpT1value;
    pDHCPMSG->OPT[i + 16] = dhcpT2value;
    pDHCPMSG->OPT[i + 17] = performRouterDiscovery;
    pDHCPMSG->OPT[i + 18] = staticRoute;

    // end option
    pDHCPMSG->OPT[i + 19] = endOption;

    DHCP_DBG("> Send DHCP_REQUEST\r\n");

    sendto(dhcp.sn, (uint8_t*)pDHCPMSG, RIP_MSG_SIZE, ip, DHCP_SERVER_PORT);
}

/* SEND DHCP DHCPDECLINE */
static void send_DHCP_DECLINE(void)
{
    //ip broadcast
    uint8_t ip[4] = { 0xff, 0xff, 0xff, 0xff };

    memset(pDHCPMSG, 0, RIP_MSG_SIZE);

    //init message head
    pDHCPMSG->op = DHCP_BOOTREQUEST;
    pDHCPMSG->htype = DHCP_HTYPE10MB;
    pDHCPMSG->hlen = DHCP_HLENETHERNET;
    pDHCPMSG->hops = DHCP_HOPS;
    pDHCPMSG->xid = htonl(dhcp.xid);
    pDHCPMSG->secs = htons(DHCP_SECS);
    pDHCPMSG->flags = htons(DHCP_FLAGSUNICAST);
    memcpy(pDHCPMSG->chaddr, dhcp.mac, 6);

    // Option Request Param.
    *((uint32_t*)pDHCPMSG->OPT) = htonl(MAGIC_COOKIE);

    pDHCPMSG->OPT[4] = dhcpMessageType;
    pDHCPMSG->OPT[5] = 0x01;
    pDHCPMSG->OPT[6] = DHCP_DECLINE;

    pDHCPMSG->OPT[7] = dhcpClientIdentifier;
    pDHCPMSG->OPT[8] = 0x07;
    pDHCPMSG->OPT[9] = 0x01;
    pDHCPMSG->OPT[10] = dhcp.mac[0];
    pDHCPMSG->OPT[11] = dhcp.mac[1];
    pDHCPMSG->OPT[12] = dhcp.mac[2];
    pDHCPMSG->OPT[13] = dhcp.mac[3];
    pDHCPMSG->OPT[14] = dhcp.mac[4];
    pDHCPMSG->OPT[15] = dhcp.mac[5];

    pDHCPMSG->OPT[16] = dhcpRequestedIPaddr;
    pDHCPMSG->OPT[17] = 0x04;
    pDHCPMSG->OPT[18] = dhcp.allocated_ip[0];
    pDHCPMSG->OPT[19] = dhcp.allocated_ip[1];
    pDHCPMSG->OPT[20] = dhcp.allocated_ip[2];
    pDHCPMSG->OPT[21] = dhcp.allocated_ip[3];

    pDHCPMSG->OPT[22] = dhcpServerIdentifier;
    pDHCPMSG->OPT[23] = 0x04;
    pDHCPMSG->OPT[24] = dhcp.sip[0];
    pDHCPMSG->OPT[25] = dhcp.sip[1];
    pDHCPMSG->OPT[26] = dhcp.sip[2];
    pDHCPMSG->OPT[27] = dhcp.sip[3];

    pDHCPMSG->OPT[28] = endOption;

    DHCP_DBG("\r\n> Send DHCP_DECLINE\r\n");
    //send broadcasting packet
    sendto(dhcp.sn, (uint8_t*)pDHCPMSG, RIP_MSG_SIZE, ip, DHCP_SERVER_PORT);
}

/* PARSE REPLY pDHCPMSG */
static int8_t parseDHCPMSG(void)
{
    uint8_t svr_addr[6];
    uint16_t svr_port;
    uint16_t len;

    uint8_t* p;
    uint8_t* e;
    uint8_t type;
    uint8_t opt_len;

    if ((len = getSn_RX_RSR(dhcp.sn)) > 0)
    {
        len = recvfrom(dhcp.sn, (uint8_t*)pDHCPMSG, len, svr_addr, &svr_port);
        //DHCP_DBG("DHCP message : %d.%d.%d.%d(%d) %d received. \r\n",svr_addr[0],svr_addr[1],svr_addr[2], svr_addr[3],svr_port, len);
    }
    else {
        return 0;
    }

    // compare mac address and port
    if (svr_port == DHCP_SERVER_PORT) {

        if (memcmp(pDHCPMSG->chaddr, dhcp.mac, 6) != 0)
            return 0;

        type = 0;
        p = pDHCPMSG->OPT + sizeof(MAGIC_COOKIE);
        e = p + (len - STRUCT_OFFSET(RIP_MSG, OPT) - sizeof(MAGIC_COOKIE));

        while (p < e) {

            switch (*p++) {

            case endOption:
                p = e;   //break
                break;
            case padOption:
                break;
            case dhcpMessageType:
                p++;
                type = *p++;
                DHCP_DBG("dhcpMessageType %d\r\n", type);
                break;
            case subnetMask:
                opt_len = *p++;
                memcpy(dhcp.allocated_sn, p, 4);
                p += opt_len;
                DHCP_DBG("subnetMask %d.%d.%d.%d\r\n", dhcp.allocated_sn[0], dhcp.allocated_sn[1], dhcp.allocated_sn[2], dhcp.allocated_sn[3]);
                break;
            case routersOnSubnet:
                opt_len = *p++;
                memcpy(dhcp.allocated_gw, p, 4);
                p += opt_len;
                DHCP_DBG("routersOnSubnet %d.%d.%d.%d\r\n", dhcp.allocated_gw[0], dhcp.allocated_gw[1], dhcp.allocated_gw[2], dhcp.allocated_gw[3]);
                break;
            case dns:
                opt_len = *p++;
                memcpy(dhcp.allocated_dns, p, 4);
                p += opt_len;
                DHCP_DBG("dns %d.%d.%d.%d\r\n", dhcp.allocated_dns[0], dhcp.allocated_dns[1], dhcp.allocated_dns[2], dhcp.allocated_dns[3]);
                break;
            case dhcpIPaddrLeaseTime:
                opt_len = *p++;
                dhcp.lease_time = ntohl(*((uint32_t*)p));
                p += opt_len;
                //dhcp.lease_time = 10;  fix debug
                DHCP_DBG("dhcpIPaddrLeaseTime %d\r\n", dhcp.lease_time);
                break;
            case dhcpServerIdentifier:
                opt_len = *p++;
                memcpy(dhcp.sip, p, 4);
                p += opt_len;
                DHCP_DBG("dhcpServerIdentifier %d.%d.%d.%d\r\n", dhcp.sip[0], dhcp.sip[1], dhcp.sip[2], dhcp.sip[3]);
                break;
            default:
                opt_len = *p++;
                p += opt_len;
                break;
            } // switch
        } // while
    } // if
    return    type;
}


/* Rset the DHCP timeout count and retry count. */
static void reset_DHCP_timeout(void)
{
    dhcp.tick_1s = 0;
    dhcp.tick_next = DHCP_WAIT_TIME;
    dhcp.retry_count = 0;
}


static uint8_t check_DHCP_timeout(void)
{
    uint8_t ret = DHCP_RUNNING;

    if (dhcp.retry_count < MAX_DHCP_RETRY) {
        if (dhcp.tick_next < dhcp.tick_1s) {

            switch (dhcp.state) {
            case STATE_DHCP_DISCOVER:
                DHCP_WRN("<<timeout>> state : STATE_DHCP_DISCOVER\r\n");
                send_DHCP_DISCOVER();
                break;

            case STATE_DHCP_REQUEST:
                DHCP_WRN("<<timeout>> state : STATE_DHCP_REQUEST\r\n");
                send_DHCP_REQUEST();
                break;

            case STATE_DHCP_REREQUEST:
                DHCP_WRN("<<timeout>> state : STATE_DHCP_REREQUEST\r\n");
                send_DHCP_REQUEST();
                break;

            default:
                break;
            }

            dhcp.tick_1s = 0;
            dhcp.tick_next = dhcp.tick_1s + DHCP_WAIT_TIME;
            dhcp.retry_count++;
        }
    }
    else { // timeout occurred

        switch (dhcp.state) {
        case STATE_DHCP_DISCOVER:
            dhcp.state = STATE_DHCP_INIT;
            ret = DHCP_FAILED;
            break;
        case STATE_DHCP_REQUEST:
        case STATE_DHCP_REREQUEST:
            send_DHCP_DISCOVER();
            dhcp.state = STATE_DHCP_DISCOVER;
            break;
        default:
            break;
        }
        reset_DHCP_timeout();
    }
    return ret;
}

static int8_t check_DHCP_leasedIP(void)
{
    uint8_t tmp;
    int32_t ret;

    //WIZchip RCR value changed for ARP Timeout count control
    tmp = getRCR();
    setRCR(0x03);

    // IP conflict detection : ARP request - ARP reply
    // Broadcasting ARP Request for check the IP conflict using UDP sendto() function
    ret = sendto(dhcp.sn, (uint8_t*)"CHECK_IP_CONFLICT", 17, dhcp.allocated_ip, 5000);

    // RCR value restore
    setRCR(tmp);

    if (ret == SOCKERR_TIMEOUT) {
        // UDP send Timeout occurred : allocated IP address is unique, DHCP Success
        DHCP_DBG("\r\n> Check leased IP - OK\r\n");

        return 1;
    }
    else {
        // Received ARP reply or etc : IP address conflict occur, DHCP Failed
        send_DHCP_DECLINE();

        ret = dhcp.tick_1s;
        while ((dhcp.tick_1s - ret) < 2);   // wait for 1s over; wait to complete to send DECLINE message;

        return 0;
    }
}


uint8_t DHCP_run(void)
{
    uint8_t  type;
    uint8_t  ret;

    if (dhcp.state == STATE_DHCP_STOP) return DHCP_STOPPED;

    if (getSn_SR(dhcp.sn) != SOCK_UDP)
        socket(dhcp.sn, Sn_MR_UDP, DHCP_CLIENT_PORT, 0x00);

    ret = DHCP_RUNNING;
    type = parseDHCPMSG();

    switch (dhcp.state) {

    case STATE_DHCP_INIT:
        memset(dhcp.allocated_ip, 0, 4);
        send_DHCP_DISCOVER();
        dhcp.state = STATE_DHCP_DISCOVER;
        break;

    case STATE_DHCP_DISCOVER:
        if (type == DHCP_OFFER) {

            DHCP_DBG("> Receive DHCP_OFFER\r\n");

            memcpy(dhcp.allocated_ip, pDHCPMSG->yiaddr, 4);
            send_DHCP_REQUEST();
            dhcp.state = STATE_DHCP_REQUEST;
        }
        else ret = check_DHCP_timeout();
        break;

    case STATE_DHCP_REQUEST:
        if (type == DHCP_ACK) {

            DHCP_DBG("> Receive DHCP_ACK\r\n");

            if (check_DHCP_leasedIP()) {
                // Network info assignment from DHCP
                dhcp.assign_ip();
                reset_DHCP_timeout();

                dhcp.state = STATE_DHCP_LEASED;
            }
            else {
                // IP address conflict occurred
                reset_DHCP_timeout();
                dhcp.conflict_ip();
                dhcp.state = STATE_DHCP_INIT;
            }
        }
        else if (type == DHCP_NAK) {

            DHCP_DBG("> Receive DHCP_NACK\r\n");

            reset_DHCP_timeout();

            dhcp.state = STATE_DHCP_DISCOVER;
        }
        else ret = check_DHCP_timeout();
        break;

    case STATE_DHCP_LEASED:
        ret = DHCP_IP_LEASED;
        if ((dhcp.lease_time != INFINITE_LEASETIME) && ((dhcp.lease_time / 2) < dhcp.tick_1s)) {

            DHCP_DBG("> Maintains the IP address \r\n");

            type = 0;
            memcpy(dhcp.old_ip, dhcp.allocated_ip, 4);

            dhcp.xid++;

            send_DHCP_REQUEST();

            reset_DHCP_timeout();

            dhcp.state = STATE_DHCP_REREQUEST;
        }
        break;

    case STATE_DHCP_REREQUEST:
        ret = DHCP_IP_LEASED;
        if (type == DHCP_ACK) {
            dhcp.retry_count = 0;
            if (memcmp(dhcp.old_ip, dhcp.allocated_ip, 4) != 0)
            {
                ret = DHCP_IP_CHANGED;
                dhcp.update_ip();

                DHCP_WRN(">IP changed.\r\n");
            }
#ifdef _DHCP_DEBUG_
            else
                DHCP_DBG(">IP is continued.\r\n");
#endif                            
            reset_DHCP_timeout();
            dhcp.state = STATE_DHCP_LEASED;
        }
        else if (type == DHCP_NAK) {

            DHCP_DBG("> Receive DHCP_NACK, Failed to maintain ip\r\n");

            reset_DHCP_timeout();

            dhcp.state = STATE_DHCP_DISCOVER;
        }
        else ret = check_DHCP_timeout();
        break;
    default:
        break;
    }

    return ret;
}

void  DHCP_stop(void)
{
    close(dhcp.sn);
    dhcp.state = STATE_DHCP_STOP;
}


void DHCP_init(uint8_t dhcp_sock, uint8_t* buf)
{
    getSHAR(dhcp.mac);
    if ((dhcp.mac[0] | dhcp.mac[1] | dhcp.mac[2] | dhcp.mac[3] | dhcp.mac[4] | dhcp.mac[5]) == 0x00)
    {
        // assing temporary mac address, you should be set SHAR before call this function. 
        dhcp.mac[0] = 0x00;
        dhcp.mac[1] = 0x08;
        dhcp.mac[2] = 0xdc;
        dhcp.mac[3] = 0x00;
        dhcp.mac[4] = 0x00;
        dhcp.mac[5] = 0x00;
        setSHAR(dhcp.mac);
        DHCP_WRN("update mac %x.%x.%x.%x.%x.%x\r\n", dhcp.mac[0], dhcp.mac[1], dhcp.mac[2], dhcp.mac[3], dhcp.mac[4], dhcp.mac[5]);
    }

    dhcp.sn = dhcp_sock;
    pDHCPMSG = (RIP_MSG*)buf;

    reset_DHCP_timeout();
    dhcp.state = STATE_DHCP_INIT;
}


void DHCP_time_handler(void)
{
    dhcp.tick_1s++;
}

void getIPfromDHCP(uint8_t* ip)
{
    memcpy(ip, dhcp.allocated_ip, 4);
}

void getGWfromDHCP(uint8_t* ip)
{
    memcpy(ip, dhcp.allocated_gw, 4);
}

void getSNfromDHCP(uint8_t* ip)
{
    memcpy(ip, dhcp.allocated_sn, 4);
}

void getDNSfromDHCP(uint8_t* ip)
{
    memcpy(ip, dhcp.allocated_dns, 4);
}

uint32_t getDHCPLeasetime(void)
{
    return dhcp.lease_time;
}
