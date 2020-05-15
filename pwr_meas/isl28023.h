/**
  ******************************************************************************
  * @file           : ISL28023.h
  * @brief          : definitions for the digital power monitor (DPM) ISL28023
  ******************************************************************************
  * @attention
  *
  * @author    Alexander Kaineder
  * @date      Jul 29, 2019
  *
  ******************************************************************************
  */

#ifndef ISL28023_H_
#define ISL28023_H_

#include "main.h"


/* Device register definition -------------------------------------------------------*/
// device info
#define REG_CAPABILITY			0x19
#define REG_VOUT_MODE			0x20
#define REG_PMBUS_REV			0x99
#define REG_IC_DEVICE_ID		0xAD
#define REG_IC_DEVICE_REV		0xAE
// global control
#define REG_RESTORE_DEFAULT		0x12
#define REG_OPERATEN			0x01
// channel control
#define REG_SET_DPM_MODE		0xD2
#define REG_DPM_CONV_SATUS		0xD3
#define REG_CONFIG_ICHANNEL		0xD4
#define REG_IOUT_CAL_GAIN		0x38
#define REG_CONFIG_VCHANNEL		0xD5
#define REG_CONFIG_PEAK_DET		0xD7
#define REG_CONFIG_EXCITATION	0xE2
// measurement register
#define REG_READ_VSHUNT_OUT		0xD6
#define REG_READ_VOUT			0x8B
#define REG_READ_IOUT			0x8C
#define REG_READ_PEAK_MIN_IOUT	0xD8
#define REG_READ_PEAK_MAX_IOUT 	0xD9
#define REG_READ_POUT			0x96
#define REG_READ_VSHUNT_OUT_AUX	0xE0
#define REG_READ_VOUT_AUX		0xE1
#define REG_READ_TEMPERATURE_1	0x8D
// threshold detector
#define REG_VOUT_OV_THRES_SET	0xDA
#define REG_VOUT_UV_THRES_SET	0xDB
#define REG_IOUT_OC_THRES_SET	0xDC
// smb alert
#define REG_CONFIG_INTR			0xDD
#define REG_FORCE_FEEDTHR_ALERT	0xDE
#define REG_SMBALERT_MASK		0x1B
#define REG_SMBALERT2_MASK		0xDF
#define REG_CLEAR_FAULTS		0x03
#define REG_STATUS_VOUT			0x7A
#define REG_STATUS_IOUT			0x7B
#define REG_STATUS_TEMPERATURE	0x7D
#define REG_STATUS_CML			0x7E
#define REG_STATUS_BYTE			0x78
#define REG_STATUS_WORD			0x79
// voltage margin
#define REG_CONFIG_VOL_MARGIN	0xE4
#define REG_SET_VOL_MARGIN		0xE3
// external clock control
#define REG_CONFIG_EXT_CLK		0xE5

/* Register value definition -------------------------------------------------------*/
// Conversion time in us
#define DPM_CONV_TIME_64		0x0
#define DPM_CONV_TIME_128		0x1
#define DPM_CONV_TIME_256		0x2
#define DPM_CONV_TIME_512		0x3
#define DPM_CONV_TIME_1024		0x5
#define DPM_CONV_TIME_2048		0x7
// Number of samples for average
#define DPM_AVG_SAMPL_1			0x0
#define DPM_AVG_SAMPL_2			0x1
#define DPM_AVG_SAMPL_4			0x2
#define DPM_AVG_SAMPL_8			0x3
#define DPM_AVG_SAMPL_16		0x4
#define DPM_AVG_SAMPL_32		0x5
#define DPM_AVG_SAMPL_64		0x6
#define DPM_AVG_SAMPL_128		0x7
#define DPM_AVG_SAMPL_264		0x8
#define DPM_AVG_SAMPL_512		0x9
#define DPM_AVG_SAMPL_1024		0xA
#define DPM_AVG_SAMPL_2048		0xB
#define DPM_AVG_SAMPL_4096		0xC
// Shift position for bit setting
#define DPM_AUX_AVG				10
#define DPM_AUX_CONV			7
#define DPM_PRI_AVG				3
#define DPM_PRI_CONV			0

/* Communication buffer definition -------------------------------------------------------*/
#define DPM_TX_BUFFER_LEN		128

typedef enum {
	DPM_OK,
	DPM_BUSY,
	DPM_READY,
	DPM_ERROR,
	DPM_BUFFER_FULL
} DPM_STAT;

typedef struct DPM_BUF
{
	uint8_t *data;
	uint8_t devAddr;
	uint8_t regAddr;
	uint8_t size;
	DPM_STAT status;

} DPM_BUF;

typedef struct DPM_RBUF
{
	uint8_t buffer[DPM_TX_BUFFER_LEN];					/* transmit buffer */
	uint16_t bufWp; 									/* write pointer */
	uint16_t bufRp; 									/* read pointer */
	uint16_t bufUsed; 									/* number of used bytes in buffer  */
} DPM_RBUF;

/* Definition for state-machine  ----------------------------------------------*/
typedef enum {
	SM_DPM_IDLE,
	SM_DPM_BUS_READY,
	SM_DPM_START,
	SM_DPM_BUS_FREE,
	SM_DPM_RX_READY,
	SM_DPM_RETRANS,
	SM_DPM_END
} DPM_SM;

/* Workspace definition -------------------------------------------------------*/
typedef struct WS_DPM
{
	I2C_HandleTypeDef *DpmI2cHandle;					/* i2c handle */
	DPM_RBUF Tx;
	DPM_SM smTx;										/* state-machine for data transmit */
	DPM_SM smRx;										/* state-machine for data receive */
	DPM_BUF halTx;										/* structure for data transmit */
	DPM_BUF halRx;										/* structure for data receive */

	DPM_STAT busStat;									/* I2C bus status */


//	int16_t shuntVal;									/* () measurement shunt value */
//	int16_t busVal;										/* () measurement bus value */

} WS_DPM;


/* Function prototype definition -------------------------------------------------------*/
void dpm_init(void);
void dpm_service(void);
DPM_STAT dpm_write(uint8_t devAddr, uint8_t regAddr, uint8_t *pData, uint8_t size);
DPM_STAT dpm_writeBuf(uint8_t devAddr, uint8_t regAddr, uint8_t *pData, uint8_t size);
DPM_STAT dpm_read(uint8_t devAddr, uint8_t regAddr, uint8_t *pData, uint8_t size);

void dpm_transmit(void);
void dpm_receive(void);


#endif /* ISL28023_H_ */







