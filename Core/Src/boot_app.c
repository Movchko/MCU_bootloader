/*
 * boot_app.c
 *
 *  Created on: Mar 26, 2026
 *      Author: 79099
 */
#include "main.h"
#include "boot.h"
#include "boot_app.h"

/* На STM32H523 программирование пользовательского Flash выполняется quad-word (16 байт). */
#define BOOT_WATCHDOG_QWORD_ADDR (0x08000000U + BOOTLOADER_SIZE + 0x1000U - 16U)

uint32_t ReadBootWatchDog() {
	uint32_t *val;
	val = (uint32_t *)BOOT_WATCHDOG_QWORD_ADDR;
	return *val;
}


void WriteBootWatchDog() {
//	uint32_t *val;
//	val = (uint32_t *)BOOT_WATCHDOG_QWORD_ADDR;
//	if(*val == WATCHDOG) return;
	uint32_t quad_word[4] = { WATCHDOG, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU };
	HAL_FLASH_Unlock();
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, BOOT_WATCHDOG_QWORD_ADDR, (uint32_t)quad_word);
	HAL_FLASH_Lock();

}

uint32_t ReadProgramWatchDog() {
	//uint32_t *val;
	//val = (uint32_t *)(GetMainCodeAdr() - 4);
	//return *val;
	return WATCHDOG; // пока делаем заглушку
}

void EraseCfg() {
	// TODO:: ERASE CFG

}

uint32_t GetMainCodeAdr() {
	return MAIN_APP_START_ADDR - FLASH_FOOTER_SZ; // перед главным кодом лежит его футер
}

uint32_t GetUpdateCodeAdr() {
	return UPDATE_APP_ADDR;
}

uint32_t GetDefaultCodeAdr() {
	return DEFAULT_APP_ADDR;
}

uint8_t EraseApp(FwImage type_fw) {
	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError;



	HAL_FLASH_Unlock();
	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;

	if(type_fw == MAIN) {

		EraseInitStruct.Banks = FLASH_BANK_1;
		EraseInitStruct.NbSectors     = APP_SIZE_IN_SECTRORS + 1; // стираем главное приложение и 1 сектор пустышку, в котором лежит футер
		EraseInitStruct.Sector        = START_SECTOR - 1;
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
			return HAL_FLASH_GetError ();
		}
	} else if(type_fw == UPDATE) {
		EraseInitStruct.Banks = FLASH_BANK_1;
		EraseInitStruct.NbSectors     = 32 - START_SECTOR - APP_SIZE_IN_SECTRORS;
		EraseInitStruct.Sector        = START_SECTOR + APP_SIZE_IN_SECTRORS;

		if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
			return HAL_FLASH_GetError ();
		}
		if(EraseInitStruct.NbSectors < APP_SIZE_IN_SECTRORS) { // если образ дошёл до конца банка, затираем во втором банке остаток
			EraseInitStruct.Banks = FLASH_BANK_2;
			EraseInitStruct.NbSectors     = APP_SIZE_IN_SECTRORS - (32 - START_SECTOR - APP_SIZE_IN_SECTRORS);
			EraseInitStruct.Sector        = 0;

			if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
				return HAL_FLASH_GetError ();
			}
		}

	}

	HAL_FLASH_Lock();

	return 0;
}

uint8_t SetApp(FwImage dst_type_fw, FwImage src_type_fw) {

	EraseApp(dst_type_fw);

	/* Для STM32H523 пользовательский Flash программируется quad-word (128-bit). */
	uint32_t quad_word[4];
	uint32_t total_words = (GetAppSize(src_type_fw) + 3U) / 4U;

	for(uint32_t i = 0; i < total_words; i += 4U) {
		quad_word[0] = 0xFFFFFFFFU;
		quad_word[1] = 0xFFFFFFFFU;
		quad_word[2] = 0xFFFFFFFFU;
		quad_word[3] = 0xFFFFFFFFU;

		for(uint32_t j = 0; j < 4U; j++) {
			if((i + j) < total_words) {
				quad_word[j] = GetWord(src_type_fw, i + j);
			}
		}

		if(SetWord(dst_type_fw, (int32_t)(i / 4U), &quad_word[0], 4) != 0) {
			return 1;
		}
	}

	return 0;
}


void ResetMCU() {
	NVIC_SystemReset();
}

uint32_t GetWord(FwImage type_fw, int32_t offset) {
	uint32_t word = 0;
	uint32_t addr = 0;

	switch(type_fw) {
		case MAIN:
			addr = GetMainCodeAdr();
			break;
		case UPDATE:
			addr = GetUpdateCodeAdr();
			break;
		case DEFAULT:
			addr = GetDefaultCodeAdr();
			break;
	}

	if((offset + addr) < 0)
		return 0;

	uint32_t *ptr = (void *)(addr + offset*4);
	word = *ptr;

	return word;
}

uint8_t GetByte(FwImage type_fw, int32_t offset) {
	uint8_t byte = 0;
	uint32_t addr = 0;

	switch(type_fw) {
		case MAIN:
			addr = GetMainCodeAdr();
			break;
		case UPDATE:
			addr = GetUpdateCodeAdr();
			break;
		case DEFAULT:
			addr = GetDefaultCodeAdr();
			break;
	}

	if((offset + addr) < 0)
		return 0;

	uint8_t *ptr = (void *)(addr + offset);
	byte = *ptr;

	return byte;
}

uint8_t SetWord(FwImage type_fw, int32_t offset, uint32_t* word, uint8_t num_word) {
	uint32_t addr = 0;

	switch(type_fw) {
		case MAIN:
			addr = GetMainCodeAdr();
			break;
		case UPDATE:
			addr = GetUpdateCodeAdr();
			break;
		case DEFAULT:
			addr = GetDefaultCodeAdr();
			break;
	}

	if((offset + addr) < 0)
		return 0;

	if((word == NULL) || (num_word != 4U))
		return 1;

	/* offset - индекс quad-word блока (16 байт). */
	uint32_t flash_addr = addr + ((uint32_t)offset * 16U);
	if((flash_addr & 0xFU) != 0U)
		return 1;

	HAL_FLASH_Unlock();
	HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, flash_addr, (uint32_t)word);
	HAL_FLASH_Lock();

	if(status != HAL_OK)
		return 1;

	return 0;
}

typedef void (*pFunction)(void);

void CallApplication() {
	__disable_irq();

	SysTick->CTRL = 0;
	HAL_RCC_DeInit();
	HAL_DeInit();
	__set_BASEPRI(0);
	__set_CONTROL(0);

    for (uint8_t i = 0; i < (sizeof(NVIC->ICER) / 4); i++) {
    	NVIC->ICER[i] = 0xFFFFFFFF;
  	  	NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    uint32_t appStack = (uint32_t) *((uint32_t *) MAIN_APP_START_ADDR); // minimum require
    pFunction appEntry = (pFunction) *(uint32_t *) (MAIN_APP_START_ADDR + 4); // minimum require

    SCB->VTOR = MAIN_APP_START_ADDR; // minimum require
    __set_MSP(appStack); // minimum require
    __enable_irq();

    appEntry(); // minimum require
}



