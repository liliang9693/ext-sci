/*!
 * @file DFRobot_RP2040_SUAB.cpp
 * @brief 这是一个传感器通用适配器板(Sensor Universal Adapter Board)，旨在配置适配器板参数，以及读取适配器板上各传感器的参数，具体功能如下所示：
 * @n 配置适配器板参数：
 * @n      1. 读取/设置传感器通用适配器板(Sensor Universal Adapter Board)的I2C地址，范围1~5；
 * @n      2. 读取/设置传感器通用适配器板(Sensor Universal Adapter Board)的年，月，日，时，分，秒的时间；
 * @n      3. 开启/关闭数据记录，开启数据记录后，会将传感器通用适配器板(Sensor Universal Adapter Board)上各传感器的数据以CSV格式的文件记录下来保存到FLASH中，
 * @n  用户可以通过U盘拷贝或查看该CSV记录文件，CSV文件的名字是以开启记录那刻时的年_月_日_时_分_秒的时间命名的。
 * @n      4. 开启/关闭OLED屏显示，（开启显示，是进入初始化页面还是进入关闭前的页面）
 * @n      5. 读取/设置 对应A/D，I2C1/UART1，I2C2/UART2等接口所对应的功能，及传感器的SKU, 默认配置为(A, NULL),(I2C1, NULL),(I2C2, NULL), NULL表示对应的接口上没有传感器
 * @n 读取适配器板上各传感器的参数：
 * @n      1. 获取传感器数据的"名称"，各名称之间用逗号(,)隔开;;
 * @n      2. 获取传感器数据的"值"，各值之间用逗号(,)隔开;
 * @n      3. 获取传感器数据值的单位，各单位之间用逗号(,)隔开;；
 * @n      4. 获取接入传感器的SKU；
 * @n      5. 以名称:值 单位的方式获取完整的传感器信息，各信息之间用逗号（,）隔开
 *
 * @copyright   Copyright (c) 2022 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [Arya](xue.peng@dfrobot.com)
 * @version  V1.0
 * @date  2022-07-20
 * @url https://github.com/DFRobot/DFRobot_ICG20660L
 */
#include <Arduino.h>
#include "DFRobot_RP2040_SUAB.h"
#include <Wire.h>
#if defined(NRF5) || defined(NRF52833)
#include <basic.h>

#endif
//#define RP2040_SUAB_DBG_ENABLE

#ifdef RP2040_SUAB_DBG_ENABLE
#define DEBUG_SERIAL  Serial
#define RP2040_SUAB_DBG(...) {DEBUG_SERIAL.print("["); DEBUG_SERIAL.print(__FUNCTION__); DEBUG_SERIAL.print("(): "); DEBUG_SERIAL.print(__LINE__); Serial.print(" ] "); DEBUG_SERIAL.println(__VA_ARGS__);}
#else
#define RP2040_SUAB_DBG(...)
#endif
#define IIC_MAX_TRANSFER     32     ///< I2C最大传输数据
#define CMD_START            0x00
#define CMD_SET_IF0          0x00  ///< 设置接口0命令，可以用此命令配置A&D接口的功能和SKU
#define CMD_SET_IF1          0x01  ///< 设置接口1命令，可以用此命令配置I2C1&UART1接口的功能和SKU
#define CMD_SET_IF2          0x02  ///< 设置接口2命令，可以用此命令配置I2C2&UART2接口的功能和SKU
#define CMD_READ_IF0         0x00  ///< 读取接口0的功能和SKU
#define CMD_READ_IF1         0x01  ///< 读取接口1的功能和SKU
#define CMD_READ_IF2         0x02  ///< 读取接口2的功能和SKU

#define CMD_SET_ADDR         0x03  ///< 设置I2C地址命令（此命令，设置成功后，立即生效）
#define CMD_READ_ADDR        0x03  ///< 读取I2C地址命令（此命令，设置成功后，立即生效）
#define CMD_SET_TIME         0x04  ///< 设置时间的年，月，日，时，分，秒
#define CMD_GET_TIME         0x04  ///< 获取时间的年，月，日，时，分，秒
#define CMD_RECORD_ON        0x05  ///< 启动csv记录
#define CMD_RECORD_OFF       0x06  ///< 停止CSV记录

#define CMD_SCREEN_ON        0x07  ///< 开启oled显示
#define CMD_SCREEN_OFF       0x08  ///< 关闭oled显示
#define CMD_GET_NAME         0x09  ///< 获取传感器数据名
#define CMD_GET_VALUE        0x0A  ///< 获取传感器数据值
#define CMD_GET_UNIT         0x0B  ///< 获取传感器数据单位
#define CMD_GET_SKU          0x0C  ///<获取传感器的SKU, SKU之间用逗号(,)分开
#define CMD_GET_INFO         0x0D  ///< 获取传感器的数据名，值和单位名，值和单位名之间空一格，其他用逗号(,)分开

#define CMD_GET_KEY_VALUE0    0x0E  ///< 根据数据名获取对应的数据的值
#define CMD_GET_KEY_VALUE1    0x0F  ///< 根据数据名获取选中的接口对应的数据的值
#define CMD_GET_KEY_VALUE2    0x10  ///< 根据数据名获取选中的接口上指定SKU对应的数据的值
#define CMD_GET_KEY_UINT0     0x11  ///< 根据数据名获取对应的数据的单位
#define CMD_GET_KEY_UINT1     0x12  ///< 根据数据名获取选中的接口对应的数据的单位
#define CMD_GET_KEY_UINT2     0x13  ///< 根据数据名获取选中的接口上指定SKU对应的数据的单位
#define CMD_RESET             0x14  ///< 复制I2C从机发送缓存命令
#define CMD_SKU_A             0x15  ///< 获取传感器转接板支持的Analog传感器SKU命令
#define CMD_SKU_D             0x16  ///< 获取传感器转接板支持的Digital传感器SKU命令
#define CMD_SKU_IIC           0x17  ///< 获取传感器转接板支持的I2C传感器SKU命令
#define CMD_SKU_UART          0x18  ///< 获取传感器转接板支持的UART传感器SKU命令

#define CMD_END             CMD_SKU_UART

#define STATUS_SUCCESS      0x53  ///< 响应成功状态   
#define STATUS_FAILED       0x63  ///< 响应失败状态 

#define DEBUG_TIMEOUT_MS    2000

#define ERR_CODE_NONE               0x00 ///< 通信正常
#define ERR_CODE_CMD_INVAILED       0x01 ///< 无效命令
#define ERR_CODE_RES_PKT            0x02 ///< 响应包错误
#define ERR_CODE_M_NO_SPACE         0x03 ///< I2C主机内存不够
#define ERR_CODE_RES_TIMEOUT        0x04 ///< 响应包接收超时
#define ERR_CODE_CMD_PKT            0x05 ///< 无效的命令包或者命令不匹配
#define ERR_CODE_SLAVE_BREAK        0x06 ///< 从机故障
#define ERR_CODE_ARGS               0x07 ///< 设置的参数错误
#define ERR_CODE_SKU                0x08 ///< 该SKU为无效SKU，或者传感器通用适配器板(Sensor Universal Adapter Board)不支持
#define ERR_CODE_S_NO_SPACE         0x09 ///< I2C从机内存不够
#define ERR_CODE_I2C_ADRESS         0x0A ///< I2C地址无效
#if defined(ESP32)
#define I2C_ACHE_MAX_LEN            32//128
#else
#define I2C_ACHE_MAX_LEN            32
#endif
typedef struct{
  uint8_t cmd;      /**< 命令                     */
  uint8_t argsNumL; /**< 命令后参数的个数低字节    */
  uint8_t argsNumH; /**< 命令后参数的个数高字节    */
  uint8_t args[0];  /**< 0长度数组，它的大小取决于上一个变量argsNumL和argsNumH的值     */
}__attribute__ ((packed)) sCmdSendPkt_t, *pCmdSendPkt_t;

typedef struct{
  uint8_t status;   /**< 响应包状态，0x53，响应成功，0x63，响应失败 */
  uint8_t cmd;      /**< 响应包命令 */
  uint8_t lenL;     /**< 除去包头的buf数组的长度的低字节 */
  uint8_t lenH;     /**< 除去包头的buf数组的长度的高字节 */
  uint8_t buf[0];   /**< 0长度数组，它的大小取决于上一个变量lenL和lenH的值 */
}__attribute__ ((packed)) sCmdRecvPkt_t, *pCmdRecvPkt_t;

DFRobot_SUAB::DFRobot_SUAB()
  :_timeout(DEBUG_TIMEOUT_MS){}

DFRobot_SUAB::~DFRobot_SUAB(){}

int DFRobot_SUAB::begin(uint32_t freq){
  return init(freq);
}

void DFRobot_SUAB::setRecvTimeout(uint32_t timeout){
  _timeout = timeout;
}

uint8_t DFRobot_SUAB::adjustRtc(const __FlashStringHelper* date, const __FlashStringHelper* time){
  char buff[11];
  memcpy_P(buff, date, 11);
  uint16_t year = conv2d(buff + 9);
  uint8_t month = 0;
  // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
  switch (buff[0]) {
      case 'J': month = buff[1] == 'a' ? 1 : month = buff[2] == 'n' ? 6 : 7; break;
      case 'F': month = 2; break;
      case 'A': month = buff[2] == 'r' ? 4 : 8; break;
      case 'M': month = buff[2] == 'r' ? 3 : 5; break;
      case 'S': month = 9; break;
      case 'O': month = 10; break;
      case 'N': month = 11; break;
      case 'D': month = 12; break;
  }
  uint8_t day = conv2d(buff + 4);
  memcpy_P(buff, time, 8);
  uint8_t hour = conv2d(buff);
  uint8_t minute = conv2d(buff + 3);
  uint8_t second = conv2d(buff + 6);

	uint8_t week = dayOfTheWeek(year, month, day);
	return adjustRtc(year,month,day,week,hour,minute,second);
}

uint8_t DFRobot_SUAB::adjustRtc(uint16_t year, uint8_t month, uint8_t day, uint8_t week, uint8_t hour, uint8_t minute, uint8_t second){
  uint8_t errorCode;
  pCmdSendPkt_t sendpkt = NULL;
  uint16_t length = sizeof(year) + sizeof(month) + sizeof(day) + sizeof(week) + sizeof(hour) + sizeof(minute) + sizeof(second);
  sendpkt           = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd      = CMD_SET_TIME;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF; 
  sendpkt->args[0]  = second;
  sendpkt->args[1]  = minute;
  sendpkt->args[2]  = hour;
  sendpkt->args[3]  = day;
  sendpkt->args[4]  = week;
  sendpkt->args[5]  = month;
  sendpkt->args[6]  = year & 0xFF;
  sendpkt->args[7]  = (year >> 8) & 0xFF;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);

  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SET_TIME, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return errorCode;
}

uint8_t DFRobot_SUAB::getRtcTime(uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *week, uint8_t *hour, uint8_t *minute, uint8_t *second){
  uint8_t errorCode;
  pCmdSendPkt_t sendpkt = NULL;
  uint16_t length = 0;
  sendpkt           = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd      = CMD_GET_TIME;
  sendpkt->argsNumL = 0;
  sendpkt->argsNumH = 0; 
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);

  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_GET_TIME, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    if(length == 8){
      if(second) *second = rcvpkt->buf[0];
      if(minute) *minute = rcvpkt->buf[1];
      if(hour)   *hour   = rcvpkt->buf[2];
      if(day)    *day    = rcvpkt->buf[3];
      if(week)   *week   = rcvpkt->buf[4];
      if(month)  *month  = rcvpkt->buf[5];
      if(year)   *year   = rcvpkt->buf[6] | (rcvpkt->buf[7] << 8);
    }
  }
  if(rcvpkt) free(rcvpkt);
  return errorCode;
}

String DFRobot_SUAB::getRtcTime(){
  uint8_t errorCode;
  String time = "";
  pCmdSendPkt_t sendpkt = NULL;
  uint16_t length = 0;
  sendpkt           = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return time;
  sendpkt->cmd      = CMD_GET_TIME;
  sendpkt->argsNumL = 0;
  sendpkt->argsNumH = 0; 
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);

  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_GET_TIME, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    if(length == 8){
      String second = "", minute = "", hour = "", day = "", week = "", month = "", year = "";
      if(rcvpkt->buf[0] < 10) second = "0";
      second += String(rcvpkt->buf[0]);

      if(rcvpkt->buf[1] < 10) minute = "0";
      minute += String(rcvpkt->buf[1]);
      
      if(rcvpkt->buf[2] < 10) hour = "0";
      hour += String(rcvpkt->buf[2]);

      if(rcvpkt->buf[3] < 10) day = "0";
      day += String(rcvpkt->buf[3]);

      week = String(rcvpkt->buf[4]);

      if(rcvpkt->buf[5] < 10) month = "0";
      month += String(rcvpkt->buf[5]);

      year = String(rcvpkt->buf[6] | (rcvpkt->buf[7] << 8));

      time = year + '/' + month + '/' + day + ' ' + week + ' ' + hour + ':' + minute + ':' + second;
    }
  }
  if(rcvpkt) free(rcvpkt);
  return time;
}

uint8_t DFRobot_SUAB::setIF0(eADIFMode_t mode){
  uint8_t errorCode;
  pCmdSendPkt_t sendpkt = NULL;
  uint16_t length = 1;
  //3.发送命令
  sendpkt           = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd      = CMD_SET_IF0;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = (uint8_t)mode;

  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);

  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SET_IF0, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return errorCode;
}

uint8_t DFRobot_SUAB::setIF0(char sku[7]){
  uint8_t errorCode;
  pCmdSendPkt_t sendpkt = NULL;
  uint16_t length = 0;
  //1.判断sku指针是否为空
  if(sku == NULL){
    RP2040_SUAB_DBG("sku pointer is NULL");
    return ERR_CODE_ARGS;
  }
  //2.判断sku是否有效
  if(!valid(sku)){
    RP2040_SUAB_DBG("sku is invaild.");
    return ERR_CODE_ARGS;
  }
  //3.发送命令
  length            = strlen(sku);
  sendpkt           = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd      = CMD_SET_IF0;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  memcpy(sendpkt->args, sku, strlen(sku));
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);

  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SET_IF0, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return errorCode;
}

uint8_t DFRobot_SUAB::setIF0(eADIFMode_t mode, char sku[7]){
  uint8_t errorCode;
  pCmdSendPkt_t sendpkt = NULL;
  uint16_t length = 0;
  //1.判断sku指针是否为空
  if(sku == NULL){
    RP2040_SUAB_DBG("sku pointer is NULL");
    return ERR_CODE_ARGS;
  }
  //2.判断sku是否有效
  if(!valid(sku)){
    RP2040_SUAB_DBG("sku is invaild.");
    return ERR_CODE_ARGS;
  }
  //3.发送命令
  length            = strlen(sku) + 1;
  sendpkt           = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd      = CMD_SET_IF0;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = (uint8_t)mode;
  memcpy(sendpkt->args + 1, sku, strlen(sku));
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);

  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SET_IF0, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return errorCode;
}

uint8_t DFRobot_SUAB::setIF1(eI2CUARTMode_t mode){
  uint8_t errorCode;
  pCmdSendPkt_t sendpkt = NULL;
  uint16_t length = 1;
  //3.发送命令
  sendpkt           = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd      = CMD_SET_IF1;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = (uint8_t)mode;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);

  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SET_IF1, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return errorCode;
}

uint8_t DFRobot_SUAB::setIF1(char sku[7]){
  uint8_t errorCode;
  pCmdSendPkt_t sendpkt = NULL;
  uint16_t length = 0;
  //1.判断sku指针是否为空
  if(sku == NULL){
    RP2040_SUAB_DBG("sku pointer is NULL");
    return ERR_CODE_ARGS;
  }
  //2.判断sku是否有效
  if(!valid(sku)){
    RP2040_SUAB_DBG("sku is invaild.");
    return ERR_CODE_ARGS;
  }
  //3.发送命令
  length            = strlen(sku);
  sendpkt           = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd      = CMD_SET_IF1;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  memcpy(sendpkt->args, sku, strlen(sku));
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);

  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SET_IF1, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return errorCode;
}

uint8_t DFRobot_SUAB::setIF1(eI2CUARTMode_t mode, char sku[7]){
  uint8_t errorCode;
  pCmdSendPkt_t sendpkt = NULL;
  uint16_t length = 0;
  //1.判断sku指针是否为空
  if(sku == NULL){
    RP2040_SUAB_DBG("sku pointer is NULL");
    return ERR_CODE_ARGS;
  }
  //2.判断sku是否有效
  if(!valid(sku)){
    RP2040_SUAB_DBG("sku is invaild.");
    return ERR_CODE_ARGS;
  }
  //3.发送命令
  length            = strlen(sku) + 1;
  sendpkt           = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd      = CMD_SET_IF1;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = (uint8_t)mode;
  memcpy(sendpkt->args + 1, sku, strlen(sku));
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);

  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SET_IF1, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return errorCode;
}

uint8_t DFRobot_SUAB::setIF2(eI2CUARTMode_t mode){
  uint8_t errorCode;
  pCmdSendPkt_t sendpkt = NULL;
  uint16_t length = 1;
  //3.发送命令
  sendpkt           = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd      = CMD_SET_IF2;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = (uint8_t)mode;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);

  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SET_IF2, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return errorCode;
}

uint8_t DFRobot_SUAB::setIF2(char sku[7]){
  uint8_t errorCode;
  pCmdSendPkt_t sendpkt = NULL;
  uint16_t length = 0;
  //1.判断sku指针是否为空
  if(sku == NULL){
    RP2040_SUAB_DBG("sku pointer is NULL");
    return ERR_CODE_ARGS;
  }
  //2.判断sku是否有效
  if(!valid(sku)){
    RP2040_SUAB_DBG("sku is invaild.");
    return ERR_CODE_ARGS;
  }
  //3.发送命令
  length            = strlen(sku);
  sendpkt           = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd      = CMD_SET_IF2;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  memcpy(sendpkt->args, sku, strlen(sku));
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);

  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SET_IF2, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return errorCode;
}

uint8_t DFRobot_SUAB::setIF2(eI2CUARTMode_t mode, char sku[7]){
  uint8_t errorCode;
  pCmdSendPkt_t sendpkt = NULL;
  uint16_t length = 0;
  //1.判断sku指针是否为空
  if(sku == NULL){
    RP2040_SUAB_DBG("sku pointer is NULL");
    return ERR_CODE_ARGS;
  }
  //2.判断sku是否有效
  if(!valid(sku)){
    RP2040_SUAB_DBG("sku is invaild.");
    return ERR_CODE_ARGS;
  }
  //3.发送命令
  length            = strlen(sku) + 1;
  sendpkt           = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd      = CMD_SET_IF2;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = (uint8_t)mode;
  memcpy(sendpkt->args + 1, sku, strlen(sku));
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);

  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SET_IF2, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return errorCode;
}

String DFRobot_SUAB::getSensorModeDescribe(eADIFMode_t mode){
  switch(mode){
    case eAnalogMode:
         return "ANALOG";
    case eDigitalMode:
         return "DIGITAL";
  }
  return "UNKNOWN";
}

String DFRobot_SUAB::getSensorModeDescribe(eI2CUARTMode_t mode){
  switch(mode){
    case eI2CMode:
         return "I2C";
    case eUARTMode:
         return "UART";
  }
  return "UNKNOWN";
}

String DFRobot_SUAB::getIF0Config(eADIFMode_t *mode){
  String config;

  uint8_t errorCode;
  uint16_t length = 0;
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t));
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_READ_IF0;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_READ_IF0, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    if(mode) *mode = (eADIFMode_t)rcvpkt->buf[0];
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char sku[length];
    memcpy(sku, rcvpkt->buf + 1, length - 1);
    sku[length - 1] = '\0';
    config = String(sku);
  }
  if(rcvpkt) free(rcvpkt);
  return config;
}

String DFRobot_SUAB::getIF1Config(eI2CUARTMode_t *mode){
  String config;

  uint8_t errorCode;
  uint16_t length = 0;
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t));
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_READ_IF1;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_READ_IF1, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    if(mode) *mode = (eI2CUARTMode_t)rcvpkt->buf[0];
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char sku[length];
    memcpy(sku, rcvpkt->buf + 1, length - 1);
    sku[length - 1] = '\0';
    config = String(sku);
  }
  if(rcvpkt) free(rcvpkt);

  return config;
}

String DFRobot_SUAB::getIF2Config(eI2CUARTMode_t *mode){
  String config = "";

  uint8_t errorCode;
  uint16_t length = 0;
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t));
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_READ_IF2;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_READ_IF2, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    if(mode) *mode = (eI2CUARTMode_t)rcvpkt->buf[0];
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char sku[length];
    memcpy(sku, rcvpkt->buf + 1, length - 1);
    sku[length - 1] = '\0';
    config = String(sku);
  }
  if(rcvpkt) free(rcvpkt);
  return config;
}

uint8_t DFRobot_SUAB::enableRecord(){
  String config;

  uint8_t errorCode;
  uint16_t length = 0;
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t));
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd = CMD_RECORD_ON;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_RECORD_ON, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return errorCode;

}
uint8_t DFRobot_SUAB::disableRecord(){
  String config;

  uint8_t errorCode;
  uint16_t length = 0;
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t));
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd = CMD_RECORD_OFF;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_RECORD_OFF, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return errorCode;
}

uint8_t DFRobot_SUAB::oledScreenOn(){
  String config;

  uint8_t errorCode;
  uint16_t length = 0;
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t));
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd = CMD_SCREEN_ON;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SCREEN_ON, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return errorCode;
}

uint8_t DFRobot_SUAB::oledScreenOff(){
  String config;

  uint8_t errorCode;
  uint16_t length = 0;
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t));
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd = CMD_SCREEN_OFF;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SCREEN_OFF, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return errorCode;
}

String DFRobot_SUAB::getKeys(eInterfaceList_t inf){
  return getKeys((uint8_t)inf);
}

String DFRobot_SUAB::getKeys(uint8_t inf){
  String name = "";

  uint8_t errorCode;
  uint16_t length = 1;
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_GET_NAME;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = inf;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_GET_NAME, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char sku[length + 1];
    memcpy(sku, rcvpkt->buf, length);
    sku[length] = '\0';
    name = String(sku);
  }
  if(rcvpkt) free(rcvpkt);
  return name;
}

String DFRobot_SUAB::getValues(eInterfaceList_t inf){
  return getValues((uint8_t)inf);
}

String DFRobot_SUAB::getValues(uint8_t inf){
  String values = "";

  uint8_t errorCode;
  uint16_t length = 1;
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_GET_VALUE;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = inf;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_GET_VALUE, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char sku[length + 1];
    memcpy(sku, rcvpkt->buf, length);
    sku[length] = '\0';
    values = String(sku);
  }
  if(rcvpkt) free(rcvpkt);
  return values;
}

String DFRobot_SUAB::getUnits(eInterfaceList_t inf){
  return getUnits((uint8_t)inf);
}

String DFRobot_SUAB::getUnits(uint8_t inf){
  String unit = "";

  uint8_t errorCode;
  uint16_t length = 1;
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_GET_UNIT;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = inf;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_GET_UNIT, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char sku[length + 1];
    memcpy(sku, rcvpkt->buf, length);
    sku[length] = '\0';
    unit = String(sku);
  }
  if(rcvpkt) free(rcvpkt);
  return unit;
}

String DFRobot_SUAB::getInformation(eInterfaceList_t inf){
  return getInformation((uint8_t)inf);
}

String DFRobot_SUAB::getInformation(uint8_t inf){
  String info = "";

  uint8_t errorCode;
  uint16_t length = 1;
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_GET_INFO;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = inf;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_GET_INFO, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char sku[length + 1];
    memcpy(sku, rcvpkt->buf, length);
    sku[length] = '\0';
    info = String(sku);
  }
  if(rcvpkt) free(rcvpkt);
  return info;
}

String DFRobot_SUAB::getSKU(eInterfaceList_t inf){
  return getSKU((uint8_t)inf);
}

String DFRobot_SUAB::getSKU(uint8_t inf){
  String skus = "";

  uint8_t errorCode;
  uint16_t length = 1;
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_GET_SKU;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = inf;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_GET_SKU, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char sku[length + 1];
    memcpy(sku, rcvpkt->buf, length);
    sku[length] = '\0';
    skus = String(sku);
  }
  if(rcvpkt) free(rcvpkt);
  return skus;
}

String DFRobot_SUAB::getValue(char *keys){
  String values = "";

  uint8_t errorCode;
  uint16_t length = 0;
  if(keys == NULL) return values;
  length = strlen(keys);

  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_GET_KEY_VALUE0;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  memcpy(sendpkt->args, keys, strlen(keys));
  
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_GET_KEY_VALUE0, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char sku[length + 1];
    memcpy(sku, rcvpkt->buf, length);
    sku[length] = '\0';
    values = String(sku);
  }
  if(rcvpkt) free(rcvpkt);
  return values;
}

String DFRobot_SUAB::getValue(eInterfaceList_t inf, char *keys){
  return getValue((uint8_t)inf, keys);
}

String DFRobot_SUAB::getValue(uint8_t inf, char *keys){
  String values = "";

  uint8_t errorCode;
  uint16_t length = 0;
  if(keys == NULL) return values;
  length = strlen(keys) + 1;

  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_GET_KEY_VALUE1;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = inf;
  memcpy(sendpkt->args + 1, keys, strlen(keys));

  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_GET_KEY_VALUE1, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char sku[length + 1];
    memcpy(sku, rcvpkt->buf, length);
    sku[length] = '\0';
    values = String(sku);
  }
  if(rcvpkt) free(rcvpkt);
  return values;
}

String DFRobot_SUAB::getValue(eInterfaceList_t inf, char sku[7], char *keys){
  return getValue((uint8_t)inf, sku, keys);
}

String DFRobot_SUAB::getValue(uint8_t inf, char sku[7], char *keys){
  String values = "";

  uint8_t errorCode;
  uint16_t length = 0;
  if(keys == NULL) return values;
  if(valid(sku)) return values;
  length = strlen(keys) + 7 + 1;

  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_GET_KEY_VALUE2;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = inf;
  memcpy(sendpkt->args + 1, sku, 7);
  memcpy(sendpkt->args + 8, keys, strlen(keys));

  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_GET_KEY_VALUE2, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char sku[length + 1];
    memcpy(sku, rcvpkt->buf, length);
    sku[length] = '\0';
    values = String(sku);
  }
  if(rcvpkt) free(rcvpkt);
  return values;
}

String DFRobot_SUAB::getUnit(char *keys){
  String values = "";

  uint8_t errorCode;
  uint16_t length = 0;
  if(keys == NULL) return values;
  length = strlen(keys);

  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_GET_KEY_UINT0;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  memcpy(sendpkt->args, keys, strlen(keys));

  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_GET_KEY_UINT0, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char sku[length + 1];
    memcpy(sku, rcvpkt->buf, length);
    sku[length] = '\0';
    values = String(sku);
  }
  if(rcvpkt) free(rcvpkt);
  return values;
}
String DFRobot_SUAB::getUnit(eInterfaceList_t inf, char *keys){
  return getUnit((uint8_t)inf, keys);
}

String DFRobot_SUAB::getUnit(uint8_t inf, char *keys){
  String values = "";

  uint8_t errorCode;
  uint16_t length = 0;
  if(keys == NULL) return values;
  length = strlen(keys) + 1;

  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_GET_KEY_UINT1;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = inf;
  memcpy(sendpkt->args + 1, keys, strlen(keys));

  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_GET_KEY_UINT1, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char sku[length + 1];
    memcpy(sku, rcvpkt->buf, length);
    sku[length] = '\0';
    values = String(sku);
  }
  if(rcvpkt) free(rcvpkt);
  return values;
}


String DFRobot_SUAB::getUnit(eInterfaceList_t inf, char sku[7], char *keys){
  return getUnit((uint8_t)inf, sku, keys);
}

String DFRobot_SUAB::getUnit(uint8_t inf, char sku[7], char *keys){
  String values = "";

  uint8_t errorCode;
  uint16_t length = 0;
  if(keys == NULL) return values;
  if(valid(sku)) return values;
  length = strlen(keys) + 7 + 1;

  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_GET_KEY_UINT2;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = inf;
  memcpy(sendpkt->args + 1, sku, 7);
  memcpy(sendpkt->args + 8, keys, strlen(keys));

  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_GET_KEY_UINT2, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char sku[length + 1];
    memcpy(sku, rcvpkt->buf, length);
    sku[length] = '\0';
    values = String(sku);
  }
  if(rcvpkt) free(rcvpkt);
  return values;
}

String DFRobot_SUAB::getAnalogSensorSKU(){
  String sku = "";

  uint8_t errorCode;
  uint16_t length = 0;

  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_SKU_A;
  sendpkt->argsNumL = 0;
  sendpkt->argsNumH = 0;

  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SKU_A, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char buf[length + 1];
    memcpy(buf, rcvpkt->buf, length);
    buf[length] = '\0';
    sku = String(buf);
  }
  if(rcvpkt) free(rcvpkt);
  return sku;
}

String DFRobot_SUAB::getDigitalSensorSKU(){
  String sku = "";

  uint8_t errorCode;
  uint16_t length = 0;

  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_SKU_D;
  sendpkt->argsNumL = 0;
  sendpkt->argsNumH = 0;

  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SKU_D, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char buf[length + 1];
    memcpy(buf, rcvpkt->buf, length);
    buf[length] = '\0';
    sku = String(buf);
  }
  if(rcvpkt) free(rcvpkt);
  return sku;
}

String DFRobot_SUAB::getI2CSensorSKU(){
  String sku = "";

  uint8_t errorCode;
  uint16_t length = 0;

  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_SKU_IIC;
  sendpkt->argsNumL = 0;
  sendpkt->argsNumH = 0;

  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SKU_IIC, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char buf[length + 1];
    memcpy(buf, rcvpkt->buf, length);
    buf[length] = '\0';
    sku = String(buf);
  }
  if(rcvpkt) free(rcvpkt);
  return sku;
}

String DFRobot_SUAB::getUARTSensorSKU(){
  String sku = "";

  uint8_t errorCode;
  uint16_t length = 0;

  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return "";
  sendpkt->cmd = CMD_SKU_UART;
  sendpkt->argsNumL = 0;
  sendpkt->argsNumH = 0;

  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SKU_UART, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)){
    length = (rcvpkt->lenH << 8) | rcvpkt->lenL;
    char buf[length + 1];
    memcpy(buf, rcvpkt->buf, length);
    buf[length] = '\0';
    sku = String(buf);
  }
  if(rcvpkt) free(rcvpkt);
  return sku;
}

void DFRobot_SUAB::reset(uint8_t cmd){
  uint8_t errorCode;
  uint16_t length = 1;
  if(cmd > CMD_RESET) return;
  
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + 1);
  if(sendpkt == NULL) return;
  sendpkt->cmd = CMD_RESET;
  sendpkt->argsNumL = 1;
  sendpkt->argsNumH = 0;
  sendpkt->args[0]  = cmd;

  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  delay(1000);
}

// 00：SEN、01：DFR、10：TEL
bool DFRobot_SUAB::valid(char *sku){
  // 11: 
  bool ret = false;
  if(sku == NULL) return false;

  switch(strlen(sku)){
    case 4:
         //如果SKU为大写NULL，则返回true
         if(strncmp(sku,"NULL",4) == 0) ret = true;
        break;
    case 7:
        {
          if((strncmp(sku,"SEN",3) == 0) || (strncmp(sku,"DFR",3) == 0) || (strncmp(sku,"TEL",3) == 0)){
            int a = String(sku[3]).toInt();
            int b = String(sku[4]).toInt();
            int c = String(sku[5]).toInt();
            int d = String(sku[6]).toInt();
            if(((a <= 9) && (a >= 0)) && ((b <= 9) && (b >= 0)) && ((c <= 9) && (c >= 0)) && ((d <= 9) && (d >= 0))) ret = true;
          }
        }
        break;
  }

  return ret;
}


void * DFRobot_SUAB::recvPacket(uint8_t cmd, uint8_t *errorCode){
  if(cmd > CMD_END){
    RP2040_SUAB_DBG("cmd is error!");
    if(errorCode) *errorCode = ERR_CODE_CMD_INVAILED; //没有这个命令
    return NULL;
  }
  
  sCmdRecvPkt_t recvPkt;
  pCmdRecvPkt_t recvPktPtr = NULL;
  uint16_t length = 0;
  uint32_t t = millis();
  while(millis() - t < _timeout/*time_ms*/){
    recvData(&recvPkt.status, 1);
    switch(recvPkt.status){
      case STATUS_SUCCESS:
      case STATUS_FAILED:
      {
        recvData(&recvPkt.cmd, 1);
        if(recvPkt.cmd != cmd){
          reset(cmd);
          if(errorCode) *errorCode = ERR_CODE_RES_PKT; //响应包错误
          RP2040_SUAB_DBG("Response pkt is error!");
          return NULL;
        }
        recvData(&recvPkt.lenL, 2);
        length = (recvPkt.lenH << 8) | recvPkt.lenL;
        recvPktPtr = (pCmdRecvPkt_t)malloc(sizeof(sCmdRecvPkt_t) + length);
        if(recvPktPtr == NULL){
          reset(cmd);
          if(errorCode) *errorCode = ERR_CODE_M_NO_SPACE; //I2C主机内存不够
          return NULL;
        }
        memcpy(recvPktPtr, &recvPkt, sizeof(sCmdRecvPkt_t));
        if(length) recvData(recvPktPtr->buf, length);
        if(errorCode) *errorCode = ERR_CODE_NONE;
        RP2040_SUAB_DBG(millis() - t);
        return recvPktPtr;
      }
    }
    delay(50);
    yield();
  }
  reset(cmd);
  if(errorCode) *errorCode = ERR_CODE_RES_TIMEOUT; //接收包超时
  RP2040_SUAB_DBG("Time out!");
  RP2040_SUAB_DBG(millis() - t);
  return NULL;
}

uint8_t DFRobot_SUAB::conv2d(const char* p)
{
	uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}



uint16_t DFRobot_SUAB::date2days(uint16_t year, uint8_t month, uint8_t day)
{
	if (year >= 2000)
        year -= 2000;
  uint16_t days = day;
    for (uint8_t i = 1; i < month; ++i){
        switch(i){
            case 2:
                  days += 28;
                  break;
            case 4:
            case 6:
            case 9:
            case 11:
                  days += 30;
                  break;
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                  days += 31;
                  break;
        }
    }
    if (month > 2 && year % 4 == 0)
        ++days; //闰年
    //闰年分为普通闰年和世纪闰年
    //判断方法：公历年份（阳历）是4的倍数，且不是一百的倍数，为普通闰年，公历年份是整百数，且必须是400的倍数才是世纪闰年。
    //4年一闰，百年不润，400年再闰
    //2000年是闰年，2004是闰年，2100不是闰年，2104是闰年
    return days + 365 * year + (year + 3) / 4 - 1;
}

uint8_t DFRobot_SUAB::dayOfTheWeek(uint16_t year, uint8_t month, uint8_t day)
{
	uint16_t days = date2days(year, month, day);
    return (days + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

DFRobot_RP2040_SUAB_IIC::DFRobot_RP2040_SUAB_IIC(uint8_t addr, TwoWire *pWire)
  :DFRobot_SUAB(),_pWire(pWire),_addr(addr){
  
}

DFRobot_RP2040_SUAB_IIC::~DFRobot_RP2040_SUAB_IIC(){}

int DFRobot_RP2040_SUAB_IIC::init(uint32_t freq){
  if (_pWire == NULL) return -1;
  _pWire->begin();
  _pWire->setClock(freq);
  _pWire->beginTransmission(_addr);
  if(_pWire->endTransmission() != 0) return -2;

  reset(CMD_RESET);
  return 0;
}

uint8_t DFRobot_RP2040_SUAB_IIC::setI2CAddress(uint8_t addr){

  uint8_t errorCode;
  uint16_t length = 1;
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t) + length);
  if(sendpkt == NULL) return ERR_CODE_M_NO_SPACE;
  sendpkt->cmd = CMD_SET_ADDR;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  sendpkt->args[0]  = addr;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_SET_ADDR, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  if(errorCode == ERR_CODE_NONE) _addr = addr;
  return errorCode;
}
uint8_t DFRobot_RP2040_SUAB_IIC::getI2CAddress(){

  uint8_t errorCode;
  uint8_t addr = 0;
  uint16_t length = 0;
  pCmdSendPkt_t sendpkt = NULL;
  sendpkt = (pCmdSendPkt_t)malloc(sizeof(sCmdSendPkt_t));
  if(sendpkt == NULL) return 0;
  sendpkt->cmd = CMD_READ_ADDR;
  sendpkt->argsNumL = length & 0xFF;
  sendpkt->argsNumH = (length >> 8) & 0xFF;
  length += sizeof(sCmdSendPkt_t);
  sendPacket(sendpkt, length, true);
  free(sendpkt);
  
  pCmdRecvPkt_t rcvpkt = (pCmdRecvPkt_t)recvPacket(CMD_READ_ADDR, &errorCode);
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_FAILED)) errorCode = rcvpkt->buf[0];
  if((rcvpkt != NULL) && (rcvpkt->status == STATUS_SUCCESS)) addr = rcvpkt->buf[0];
  if(rcvpkt) free(rcvpkt);
  return addr;
}

void DFRobot_RP2040_SUAB_IIC::sendPacket(void *pkt, int length, bool stop){
  uint8_t *pBuf = (uint8_t *)pkt;
  int remain = length;
  if((pkt == NULL) || (length == 0)) return;
  _pWire->beginTransmission(_addr);
  while(remain){
    length = (remain > IIC_MAX_TRANSFER) ? IIC_MAX_TRANSFER : remain;
    _pWire->write(pBuf, length);
    remain -= length;
    pBuf += length;
#if defined(ESP32)
    if(remain) _pWire->endTransmission(true);
#else
    if(remain) _pWire->endTransmission(false);
#endif
  }
  _pWire->endTransmission(stop);
}

int DFRobot_RP2040_SUAB_IIC::recvData(void *data, int len){
  uint8_t *pBuf = (uint8_t *)data;
  int remain = len;
  int total = 0;
  if(pBuf == NULL){
    RP2040_SUAB_DBG("pBuf ERROR!! : null pointer");
    return 0;
  }
  
  while(remain){
    len = remain > I2C_ACHE_MAX_LEN ? I2C_ACHE_MAX_LEN : remain;
    remain -= len;
#if defined(ESP32)
    if(remain) _pWire->requestFrom(_addr, len, true);
#else
    if(remain) _pWire->requestFrom(_addr, len, false);
#endif
    else _pWire->requestFrom(_addr, len, true);
    for(int i = 0; i < len; i++){
      pBuf[i] = _pWire->read();
      RP2040_SUAB_DBG(pBuf[i],HEX);
      yield();
    }
    pBuf += len;
    total += len;
  }
  return total;
}

void DFRobot_RP2040_SUAB_IIC::recvFlush(){
  while(_pWire->available()){
    _pWire->read();
    yield();
  }
}

void DFRobot_RP2040_SUAB_IIC::sendFlush(){
  _pWire->flush();
}