/**
  ******************************************************************************
  * @file           : ISL28023.c
  * @brief          : functions for communication with the power monitor
  * 					ISL28023
  ******************************************************************************
  * @attention
  *
  * @author    Alexander Kaineder
  * @date      Jul 29, 2019
  *
  ******************************************************************************
  */

#include "isl28023.h"
#include "i2c_hal.h"
//#include "Stm_Platform.h"

WS_DPM WsDpm;

/**
  * @brief Initialization of I2C handle
  */
void dpm_init(struct i2c_dev_s *i2c_dev)
{
	WsDpm.DpmI2cHandle = i2c_dev->hi2c;		/* DPM I2C handle = SMBUS2 handle */
	WsDpm.Tx.bufRp = 0;						/* init tx buffer */
	WsDpm.Tx.bufWp = 0;
	WsDpm.Tx.bufUsed = 0;
	WsDpm.smTx = SM_DPM_IDLE;
	WsDpm.busStat = DPM_READY;
}

/**
  * @brief Service routine for I2C data transfer
  * 	   This routine calls the transmit and receive handler.
  * 	   The routine has to be called periodically from the main loop at high rate.
  * @retval none
  */
void dpm_service(void)
{
	dpm_transmit();

	dpm_receive();

}

/**
  * @brief Write configuration data to dpm in non blocking mode.
  *        This function writes n bytes to the device addressed with devAddr.
  *        The data is written to a circular buffer which is automatically
  *        transferred to the DPM by "dpm_transmit()".
  * @param phost: I2C handle pointer defined in "dpm_init"
  * @param devNum: Device number
  * @param devAddr: Device address
  * @param regAddr: Register address
  * @param buffer: Pointer to transmit data buffer
  * @param size: Number of bytes to be transferred (regAddr not included)
  * @retval status
  */
DPM_STAT dpm_writeBuf(uint8_t devAddr, uint8_t regAddr, uint8_t *pData, uint8_t size)
{
	int i_n;
	if (size+1 > DPM_TX_BUFFER_LEN-1 - WsDpm.Tx.bufUsed)
		return DPM_BUFFER_FULL;

	WsDpm.Tx.buffer[WsDpm.Tx.bufWp++] = devAddr;
	WsDpm.Tx.bufUsed++;
	if (WsDpm.Tx.bufWp >= DPM_TX_BUFFER_LEN)
		WsDpm.Tx.bufWp = 0;
	WsDpm.Tx.buffer[WsDpm.Tx.bufWp++] = size+1;
	WsDpm.Tx.bufUsed++;
	if (WsDpm.Tx.bufWp >= DPM_TX_BUFFER_LEN)
		WsDpm.Tx.bufWp = 0;
	WsDpm.Tx.buffer[WsDpm.Tx.bufWp++] = regAddr;
	WsDpm.Tx.bufUsed++;
	if (WsDpm.Tx.bufWp >= DPM_TX_BUFFER_LEN)
		WsDpm.Tx.bufWp = 0;
	for (i_n = 0; i_n < size; i_n++)
	{
		WsDpm.Tx.buffer[WsDpm.Tx.bufWp++] = pData[i_n];
		WsDpm.Tx.bufUsed++;
		if (WsDpm.Tx.bufWp >= DPM_TX_BUFFER_LEN)
			WsDpm.Tx.bufWp = 0;
	}
	return DPM_OK;
}

/**
  * @brief Write configuration data to dpm in blocking mode.
  *        This function writes n bytes to the device addressed with devAddr.
  * @param phost: I2C handle pointer defined in "dpm_init"
  * @param devNum: Device number
  * @param devAddr: Device address
  * @param regAddr: Register address
  * @param buffer: Pointer to transmit data buffer
  * @param size: Number of bytes to be transferred (regAddr not included)
  * @retval status
  */
DPM_STAT dpm_write(uint8_t devAddr, uint8_t regAddr, uint8_t *pData, uint8_t size)
{
	int i_n;
	uint8_t data[5];

	WsDpm.busStat = DPM_BUSY;
	data[0] = regAddr;
	for (i_n = 0; i_n < size; i_n++)
		data[i_n+1] = pData[i_n];
	if (HAL_I2C_Master_Transmit(WsDpm.DpmI2cHandle, devAddr, data, size+1, 100) != HAL_OK)
	{
		WsDpm.busStat = DPM_READY;
		return DPM_ERROR;
	}
	while (HAL_I2C_GetState(WsDpm.DpmI2cHandle) != HAL_I2C_STATE_READY) {}

	WsDpm.busStat = DPM_READY;
	return DPM_OK;
}
/**
  * @brief Read measurement data from dpm in non blocking mode.
  *        This function reads n bytes from the device addressed with devAddr.
  *        The device address and register address is written to "WsDpm.halRx.".
  *        When the Rx state-machine is in idle mode, it is directly started
  *        and the data is transferred by "dpm_receive" when the i2c bus is free.
  *        After the transfer is finished the status is in "WsDpm.halRx.status".
  * @param phost: I2C handle pointer defined in "dpm_init"
  * @param devNum: Device number
  * @param devAddr: Device address
  * @param regAddr: Register address
  * @param buffer: Pointer to receive data buffer
  * @param size: Number of bytes to be transferred (regAddr not included)
  * @retval status
  */
DPM_STAT dpm_read(uint8_t devAddr, uint8_t regAddr, uint8_t *pData, uint8_t size)
{
	if ((WsDpm.smRx == SM_DPM_IDLE) && (WsDpm.busStat == DPM_READY))
	{
		WsDpm.halRx.devAddr = devAddr;
		WsDpm.halRx.size = size;
		WsDpm.halRx.regAddr = regAddr;
		WsDpm.halRx.data = pData;
		WsDpm.halRx.status = DPM_BUSY;
		WsDpm.smRx = SM_DPM_BUS_READY;
		WsDpm.busStat = DPM_BUSY;
		return DPM_OK;
	}
	else
	{
		return DPM_BUSY;
	}
}


/**
  * @brief Receive data from DPM
  *        This function controls the transfer of the Tx.buffer content to the I2C hal routines.
  * @retval none
  */
void dpm_receive(void)
{
	switch (WsDpm.smRx)
	{
	case SM_DPM_IDLE:
		break;

	case SM_DPM_BUS_READY:
		if (HAL_I2C_GetState(WsDpm.DpmI2cHandle) == HAL_I2C_STATE_READY)
			WsDpm.smRx = SM_DPM_START;
		break;

	case SM_DPM_START:
//		if (HAL_I2C_Mem_Read(WsDpm.DpmI2cHandle, WsDpm.halRx.devAddr, WsDpm.halRx.regAddr, I2C_MEMADD_SIZE_8BIT, WsDpm.halRx.data, WsDpm.halRx.size, 100) == HAL_OK)
		if (HAL_I2C_Mem_Read_DMA(WsDpm.DpmI2cHandle, WsDpm.halRx.devAddr, WsDpm.halRx.regAddr, I2C_MEMADD_SIZE_8BIT, WsDpm.halRx.data, WsDpm.halRx.size) == HAL_OK)
			WsDpm.smRx = SM_DPM_RX_READY;
		else
		{
			WsDpm.halRx.status = DPM_ERROR;
			WsDpm.smRx = SM_DPM_END;
		}
		break;

	case SM_DPM_RX_READY:
		if (HAL_I2C_GetState(WsDpm.DpmI2cHandle) == HAL_I2C_STATE_READY)
		{
			WsDpm.halRx.status = DPM_READY;
			WsDpm.smRx = SM_DPM_END;
		}
		break;

	case SM_DPM_END:
		WsDpm.busStat = DPM_READY;
		WsDpm.smRx = SM_DPM_IDLE;
		break;

	default:
		WsDpm.smRx = SM_DPM_IDLE;
		break;
	}
}


/**
  * @brief Transmit commands from Tx.buffer
  *        This function controls the transfer of the Tx.buffer content to the I2C hal routines.
  * @retval none
  */
void dpm_transmit(void)
{
	int i_n;
	uint8_t data[5];

	switch (WsDpm.smTx)
	{
	case SM_DPM_IDLE:
		if (WsDpm.Tx.bufUsed > 0 && WsDpm.busStat == DPM_READY)
		{
			WsDpm.busStat = DPM_BUSY;
			WsDpm.smTx = SM_DPM_BUS_READY;
		}
		break;

	case SM_DPM_BUS_READY:
		if (HAL_I2C_GetState(WsDpm.DpmI2cHandle) == HAL_I2C_STATE_READY)
			WsDpm.smTx = SM_DPM_START;
		break;

	case SM_DPM_START:
		WsDpm.halTx.devAddr = WsDpm.Tx.buffer[WsDpm.Tx.bufRp++];
		WsDpm.Tx.bufUsed--;
		if (WsDpm.Tx.bufRp >= DPM_TX_BUFFER_LEN)
			WsDpm.Tx.bufRp = 0;
		WsDpm.halTx.size = WsDpm.Tx.buffer[WsDpm.Tx.bufRp++];
		WsDpm.Tx.bufUsed--;
		if (WsDpm.Tx.bufRp >= DPM_TX_BUFFER_LEN)
			WsDpm.Tx.bufRp = 0;
		WsDpm.halTx.data = data;
		for (i_n = 0; i_n < WsDpm.halTx.size; i_n++)
		{
			WsDpm.halTx.data[i_n] = WsDpm.Tx.buffer[WsDpm.Tx.bufRp++];
			WsDpm.Tx.bufUsed--;
			if (WsDpm.Tx.bufRp >= DPM_TX_BUFFER_LEN)
				WsDpm.Tx.bufRp = 0;
		}
//		if (HAL_I2C_Master_Transmit_DMA(WsDpm.DpmI2cHandle, WsDpm.halTx.devAddr, WsDpm.halTx.data, WsDpm.halTx.size) == HAL_OK)
		if (HAL_I2C_Master_Transmit(WsDpm.DpmI2cHandle, WsDpm.halTx.devAddr, WsDpm.halTx.data, WsDpm.halTx.size, 100) == HAL_OK)
			WsDpm.smTx = SM_DPM_END;
		else
			WsDpm.smTx = SM_DPM_BUS_FREE;
		break;

	case SM_DPM_BUS_FREE:
		if (HAL_I2C_GetState(WsDpm.DpmI2cHandle) == HAL_I2C_STATE_READY)
			WsDpm.smTx = SM_DPM_RETRANS;
		break;

	case SM_DPM_RETRANS:
//		if (HAL_I2C_Master_Transmit_DMA(WsDpm.DpmI2cHandle, WsDpm.halTx.devAddr, WsDpm.halTx.data, WsDpm.halTx.size) == HAL_OK)
		if (HAL_I2C_Master_Transmit(WsDpm.DpmI2cHandle, WsDpm.halTx.devAddr, WsDpm.halTx.data, WsDpm.halTx.size, 100) == HAL_OK)
			WsDpm.smTx = SM_DPM_END;
		else
			WsDpm.smTx = SM_DPM_END;
		break;

	case SM_DPM_END:
		WsDpm.busStat = DPM_READY;
		WsDpm.smTx = SM_DPM_IDLE;
		break;

	default:
		break;
	}
}

//	HAL_I2C_StateTypeDef HAL_I2C_GetState(I2C_HandleTypeDef *hi2c);
//	HAL_I2C_ModeTypeDef  HAL_I2C_GetMode(I2C_HandleTypeDef *hi2c);

//	HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
//	HAL_StatusTypeDef HAL_I2C_Master_Transmit_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
//	HAL_StatusTypeDef HAL_I2C_Master_Transmit_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
//	HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
//	HAL_StatusTypeDef HAL_I2C_Master_Receive_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
//	HAL_StatusTypeDef HAL_I2C_Master_Receive_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
//	HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
//	HAL_StatusTypeDef HAL_I2C_Mem_Read_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
//	HAL_StatusTypeDef HAL_I2C_Mem_Read_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);


