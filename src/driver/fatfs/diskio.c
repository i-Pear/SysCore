/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "../sdcard.h"

/* Definitions of physical drive number for each drive */
//#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
//#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
//#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

#define SD_CARD (0)
extern uint16 SD_SECTOR_SIZE;
extern uint16 SD_BLOCK_SIZE;
extern uint32 SD_SECTOR_COUNT;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS status = STA_NOINIT;
    switch (pdrv) {
        case SD_CARD:
            status = RES_OK;
            break;
        default:
            status = STA_NOINIT;
            break;
    }
    return status;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS status = STA_NOINIT;
    switch (pdrv) {
        case SD_CARD:{
            if(sdcard_init() != 0){
                status = RES_OK;
            }else{
                status = STA_NOINIT;
            }
            break;
        }
        default:
            status = STA_NOINIT;
            break;
    }
    return status;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT status = RES_PARERR;
    switch (pdrv) {
        case SD_CARD:{
            for (UINT i = 0; i < count; ++i) {
                sdcard_read_sector(buff + SD_SECTOR_SIZE * i, (int)(sector + i));
            }
            status = RES_OK;
            break;
        }
        default:
            status = RES_PARERR;
            break;
    }
    return status;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT status = RES_PARERR;
    if(!count)return status;
    switch (pdrv) {
        case SD_CARD:{
            for (UINT i = 0;i < count; i++){
                sdcard_write_sector((uint8 *)buff + SD_SECTOR_SIZE * i, (int)(sector + i));
            }
            status = RES_OK;
            break;
        }
        default:
            status = RES_PARERR;
            break;
    }
    return status;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_ERROR;
    switch (pdrv) {
        case SD_CARD:{
            switch (cmd) {
                case GET_SECTOR_SIZE:
                    *(WORD*)buff = SD_SECTOR_SIZE;
                    break;
                case GET_BLOCK_SIZE:
                    *(DWORD*)buff = SD_BLOCK_SIZE;
                    break;
                case GET_SECTOR_COUNT:
                    *(DWORD*)buff = SD_SECTOR_COUNT;
                    break;
                case CTRL_SYNC:
                    break;
            }
            res = RES_OK;
            break;
        }
        default:
            res = RES_PARERR;
            break;
    }
    return res;
}

//获得时间
DWORD get_fattime (void)
{
    //返回当前时间戳
    return 0;
}