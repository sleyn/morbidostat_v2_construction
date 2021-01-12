/* COPYRIGHT (c) 2019 - 2020 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#include <string.h>

#include "common.h"

#define NVS_MAGIC       0x4449564F
#define NVS_MAJOR_VER   1
#define NVS_MINOR_VER   1
#define NVS_SECTORS     4
#define NVS_SECTOR_SIZE 1024
#define TAG_SIZE(len) ((sizeof(nvsItem_t) + (len) + 1) & ~1)

typedef struct
{
  UINT32 crc;
  UINT32 length; // All data in the sector
  UINT32 magic;
  UINT32 sequenceNum;
  UINT8  verMajor;
  UINT8  verMinor;
  UINT8  reserved[14];
} nvsSectorHeader_t;

typedef struct
{
  UINT16 length; // Data portion
  UINT16 tag;
  UINT8  data[0];
} nvsItem_t;

static int curSector;
static UINT8 SectBuf[NVS_SECTOR_SIZE];

static void nvs_ClearSector(int sector)
{
  nvsSectorHeader_t *hdr = (nvsSectorHeader_t *)SectBuf;

  memset(SectBuf, 0, sizeof(nvsSectorHeader_t));   
  hdr->length = sizeof(nvsSectorHeader_t);
  hdr->magic = NVS_MAGIC;
  hdr->verMajor = NVS_MAJOR_VER;
  hdr->verMinor = NVS_MINOR_VER;
  hdr->crc = chksum(SectBuf + 4, hdr->length - 4);
  LL_EEwrite(sector * NVS_SECTOR_SIZE, SectBuf, hdr->length);
}

void NVS_Init(void)
{
  int sector;
  UINT32 seqNum = 0;
  nvsSectorHeader_t *hdr = (nvsSectorHeader_t *)SectBuf;

  curSector = 0;  
  for (sector = 0; sector < NVS_SECTORS; sector++)
  {
    LL_EEread(sector * NVS_SECTOR_SIZE, SectBuf, sizeof(nvsSectorHeader_t));
    if (hdr->magic == NVS_MAGIC && 
        hdr->length <= NVS_SECTOR_SIZE && hdr->length >= sizeof(nvsSectorHeader_t) &&
        hdr->verMajor && hdr->verMajor <= NVS_MAJOR_VER)
    {
      LL_EEread(sector * NVS_SECTOR_SIZE + sizeof(nvsSectorHeader_t),
                SectBuf + sizeof(nvsSectorHeader_t),
                hdr->length - sizeof(nvsSectorHeader_t));
      if (chksum(SectBuf + 4, hdr->length - 4) == hdr->crc)
      {
        UART_PrintfMsg("NVS Sector %d, seq %ld, len %ld",
                       sector, hdr->sequenceNum, hdr->length);
        if (hdr->sequenceNum > seqNum)
        {
          seqNum = hdr->sequenceNum;
          curSector = sector;
        }
      }
      else 
      {
        UART_PrintfMsg("NVS Sector %d - bad crc", sector);
        nvs_ClearSector(sector);
      }
    }
    else
    {
      UART_PrintfMsg("NVS Sector %d - bad header", sector);
      nvs_ClearSector(sector);
    }
  }
  LL_EEread(curSector * NVS_SECTOR_SIZE, SectBuf, sizeof(nvsSectorHeader_t));
  LL_EEread(curSector * NVS_SECTOR_SIZE + sizeof(nvsSectorHeader_t),
            SectBuf + sizeof(nvsSectorHeader_t),
            hdr->length - sizeof(nvsSectorHeader_t));
}

void NVS_Clear(void)
{
  int sector;
  nvsSectorHeader_t *hdr = (nvsSectorHeader_t *)SectBuf;

  for (sector = 0; sector < NVS_SECTORS; sector++) nvs_ClearSector(sector);
  curSector = 0;  
  LL_EEread(curSector * NVS_SECTOR_SIZE, SectBuf, sizeof(nvsSectorHeader_t));
  LL_EEread(curSector * NVS_SECTOR_SIZE + sizeof(nvsSectorHeader_t),
            SectBuf + sizeof(nvsSectorHeader_t),
            hdr->length - sizeof(nvsSectorHeader_t));
}

/* Reads up to the length bytes. Returns read length or zero if tag not found */
UINT16 NVS_ReadTag(UINT16 tag, void *data, UINT16 length)
{
  nvsItem_t *item;
  UINT32 offset;
  UINT16 size;
  nvsSectorHeader_t *hdr = (nvsSectorHeader_t *)SectBuf;
   
  for (offset = sizeof(nvsSectorHeader_t); offset < hdr->length; offset += size)
  {
    item = (nvsItem_t *)(SectBuf + offset);
    size = TAG_SIZE(item->length);
    if (item->tag == tag && size + offset <= hdr->length)
    {
      if (item->length < length) length = item->length;
      memcpy(data, item->data, length);
      return(length);
    }
  }
  return(0);
}

void NVS_WriteTag(UINT16 tag, void *data, UINT16 length)
{
  nvsItem_t *item;
  UINT32 offset, i;
  UINT16 size;
  BOOL add = TRUE;
  nvsSectorHeader_t *hdr = (nvsSectorHeader_t *)SectBuf;

  for (offset = sizeof(nvsSectorHeader_t); offset < hdr->length; offset += size)
  {
    item = (nvsItem_t *)(SectBuf + offset);
    size = TAG_SIZE(item->length);
    if (size + offset > hdr->length) hdr->length = offset;
    else if (item->tag == tag)
    {
      if (item->length == length)
      {
        memcpy(item->data, data, length);
        add = FALSE;
      }
      else
      {
        for (i = offset + size; i < hdr->length; i++)
          SectBuf[i - size] = SectBuf[i];
        hdr->length -= size;
      }
    }
  }
  if (add)
  {
     size = TAG_SIZE(length);
     if (length && hdr->length + size <= NVS_SECTOR_SIZE)
     {
       item = (nvsItem_t *)(SectBuf + hdr->length);
       hdr->length += size;
       item->tag = tag;
       item->length = length;
       memcpy(item->data, data, length);
       if (length & 1) item->data[length] = 0xFF;
     }
   }
  if (++curSector == NVS_SECTORS) curSector = 0;
  hdr->verMajor = NVS_MAJOR_VER;
  hdr->verMinor = NVS_MINOR_VER;
  hdr->sequenceNum++;
  hdr->crc = chksum(SectBuf + 4, hdr->length - 4);
  LL_EEwrite(curSector * NVS_SECTOR_SIZE, SectBuf, hdr->length);
}
