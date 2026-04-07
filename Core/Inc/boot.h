
#ifndef BOOT_H_
#define BOOT_H_

#include "stdint.h"
#include "stdbool.h"
#include "string.h"


#define FLASH_MAIN_CODE_ADR     GetMainCodeAdr()
#define FLASH_UPDATE_CODE_ADR   GetUpdateCodeAdr()
#define FLASH_DEFAULT_CODE_ADR  GetDefaultCodeAdr()

#define BOOT_CRC_START 			0x1111
#define FLASH_FOOTER_SZ         64 // size footer, aligned to 16 bytes
#define WATCHDOG				0xAABBCCDD

typedef enum{
	MAIN,
	UPDATE,
	DEFAULT
} FwImage;

typedef struct _BIN_FOOTER
{
    uint32_t data_CRC32;
    uint32_t data_sz;
    uint16_t code_HW[3];
    uint8_t rev_HW;
    uint8_t ver_HW;
    uint8_t unused_reserv[44];
    uint32_t footer_CRC32;
} BIN_FOOTER;


void SetDefaultBoot();
void BootProcess();

uint32_t crc32(uint32_t crc, const void *buf, uint32_t size);
uint32_t Image_crc32(uint32_t crc, FwImage type_fw, uint32_t size, int32_t offset);

// функции ниже должны быть описаны в проекте платы
uint8_t EraseApp(FwImage type_fw);
uint8_t SetApp(FwImage dst_type_fw, FwImage src_type_fw);
void ResetMCU();
void CallApplication();
uint32_t GetMainCodeAdr();
uint32_t GetUpdateCodeAdr();
uint32_t GetDefaultCodeAdr();
uint32_t GetAppSize(FwImage type_fw);

uint32_t GetWord(FwImage fw, int32_t offset);
uint8_t GetByte(FwImage fw, int32_t offset);
uint8_t SetWord(FwImage type_fw, int32_t offset, uint32_t* word, uint8_t num_word);

uint32_t ReadBootWatchDog();
void WriteBootWatchDog();
uint32_t ReadProgramWatchDog();
void EraseCfg();

#endif /* BOOT_H_ */
