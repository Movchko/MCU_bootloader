/*
 * boot_app.h
 *
 *  Created on: Mar 26, 2026
 *      Author: 79099
 */

#ifndef INC_BOOT_APP_H_
#define INC_BOOT_APP_H_

#define BOOTLOADER_SIZE 0x8000 // 32kb - 4 sectora
#define START_SECTOR 6 // стартовый сектор главного приложения
#define APP_SIZE_IN_SECTRORS 18

#define APP_SIZE_IN_BYTES APP_SIZE_IN_SECTRORS * 8 *1024 // размер сектора 8кб
#define MAIN_APP_START_ADDR 0x800C000 // 4 сектора бут + 1 сектор пустоты для хедера. стартуем с 6 сектора

#define UPDATE_APP_ADDR MAIN_APP_START_ADDR + APP_SIZE_IN_BYTES
#define DEFAULT_APP_ADDR UPDATE_APP_ADDR + APP_SIZE_IN_BYTES


#endif /* INC_BOOT_APP_H_ */
