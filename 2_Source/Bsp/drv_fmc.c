#include "drv_fmc.h"

static int drv_fmc_range_check(uint32_t address, int size)
{
    uint32_t end_addr;

    if(size <= 0)
        return -1;

    if(address < FMC_START_ADDRESS)
        return -1;

    end_addr = address + (uint32_t)size - 1U;
    if(end_addr < address)
        return -1;

    if(end_addr > FMC_END_ADDRESS)
        return -1;

    return 0;
}

static void drv_fmc_flag_clear(void)
{
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
}

fmc_sector_info_struct fmc_sector_info_get(uint32_t addr)
{
    fmc_sector_info_struct sector_info;
    uint32_t temp = 0x00000000U;
    if((FMC_START_ADDRESS <= addr) && (FMC_END_ADDRESS >= addr)) {
        if((FMC_BANK1_START_ADDRESS > addr)) {
            /* bank0 area */
            temp = (addr - FMC_BANK0_START_ADDRESS) / SIZE_16KB;
            if(4U > temp) {
                sector_info.sector_name = (uint32_t)temp;
                sector_info.sector_num = CTL_SN(temp);
                sector_info.sector_size = SIZE_16KB;
                sector_info.sector_start_addr = FMC_BANK0_START_ADDRESS + (SIZE_16KB * temp);
                sector_info.sector_end_addr = sector_info.sector_start_addr + SIZE_16KB - 1;
            } else if(8U > temp) {
                sector_info.sector_name = 0x00000004U;
                sector_info.sector_num = CTL_SN(4);
                sector_info.sector_size = SIZE_64KB;
                sector_info.sector_start_addr = 0x08010000U;
                sector_info.sector_end_addr = 0x0801FFFFU;
            } else {
                temp = (addr - FMC_BANK0_START_ADDRESS) / SIZE_128KB;
                sector_info.sector_name = (uint32_t)(temp + 4);
                sector_info.sector_num = CTL_SN(temp + 4);
                sector_info.sector_size = SIZE_128KB;
                sector_info.sector_start_addr = FMC_BANK0_START_ADDRESS + (SIZE_128KB * temp);
                sector_info.sector_end_addr = sector_info.sector_start_addr + SIZE_128KB - 1;
            }
        } else {
            /* bank1 area */
            temp = (addr - FMC_BANK1_START_ADDRESS) / SIZE_16KB;
            if(4U > temp) {
                sector_info.sector_name = (uint32_t)(temp + 12);
                sector_info.sector_num = CTL_SN(temp + 16);
                sector_info.sector_size = SIZE_16KB;
                sector_info.sector_start_addr = FMC_BANK1_START_ADDRESS + (SIZE_16KB * temp);
                sector_info.sector_end_addr = sector_info.sector_start_addr + SIZE_16KB - 1;
            } else if(8U > temp) {
                sector_info.sector_name = 0x00000010;
                sector_info.sector_num = CTL_SN(20);
                sector_info.sector_size = SIZE_64KB;
                sector_info.sector_start_addr = 0x08110000U;
                sector_info.sector_end_addr = 0x0811FFFFU;
            } else if(64U > temp) {
                temp = (addr - FMC_BANK1_START_ADDRESS) / SIZE_128KB;
                sector_info.sector_name = (uint32_t)(temp + 16);
                sector_info.sector_num = CTL_SN(temp + 20);
                sector_info.sector_size = SIZE_128KB;
                sector_info.sector_start_addr = FMC_BANK1_START_ADDRESS + (SIZE_128KB * temp);
                sector_info.sector_end_addr = sector_info.sector_start_addr + SIZE_128KB - 1;
            } else {
                temp = (addr - FMC_BANK1_START_ADDRESS) / SIZE_256KB;
                sector_info.sector_name = (uint32_t)(temp + 20);
                sector_info.sector_num = CTL_SN(temp + 8);
                sector_info.sector_size = SIZE_256KB;
                sector_info.sector_start_addr = FMC_BANK1_START_ADDRESS + (SIZE_256KB * temp);
                sector_info.sector_end_addr = sector_info.sector_start_addr + SIZE_256KB - 1;
            }
        }
    } else {
        /* invalid address */
        sector_info.sector_name = FMC_WRONG_SECTOR_NAME;
        sector_info.sector_num = FMC_WRONG_SECTOR_NUM;
        sector_info.sector_size = FMC_INVALID_SIZE;
        sector_info.sector_start_addr = FMC_INVALID_ADDR;
        sector_info.sector_end_addr = FMC_INVALID_ADDR;
    }
    return sector_info;
}

uint32_t sector_name_to_number(uint32_t sector_name)
{
    if(11 >= sector_name) {
        return CTL_SN(sector_name);
    } else if(23 >= sector_name) {
        return CTL_SN(sector_name + 4);
    } else if(27 >= sector_name) {
        return CTL_SN(sector_name - 12);
    } else {
        return FMC_WRONG_SECTOR_NUM;
    }
}

int fmc_erase_sector_by_address(uint32_t address)
{
    fmc_sector_info_struct sector_info;
    /* get information about the sector in which the specified address is located */
    sector_info = fmc_sector_info_get(address);
    if(FMC_WRONG_SECTOR_NAME == sector_info.sector_name) {
        return -1;
    } else {
        /* unlock the flash program erase controller */
        fmc_unlock();
        /* clear pending flags */
        drv_fmc_flag_clear();
        /* wait the erase operation complete*/
        if(FMC_READY != fmc_sector_erase(sector_info.sector_num)) {
            fmc_lock();
            return -1;
        }
        /* lock the flash program erase controller */
        fmc_lock();
    }

    return 0;
}

void fmc_write_32bit_data(uint32_t address, uint16_t length, int32_t *data_32)
{
    uint32_t i;

    /* unlock the flash program erase controller */
    fmc_unlock();
    /* clear pending flags */
    drv_fmc_flag_clear();

    /* write data_32 to the corresponding address */
    for(i = 0; i < length; i++) {
        if(FMC_READY == fmc_word_program(address, data_32[i])) {
            address = address + 4;
        } else {
            break;
        }
    }
    /* lock the flash program erase controller */
    fmc_lock();
}

void fmc_read_32bit_data(uint32_t address, uint16_t length, int32_t *data_32)
{
    uint32_t i;

    for(i = 0; i < length; i++) {
        data_32[i] = *(__IO int32_t *)(uintptr_t)address;
        address = address + 4;
    }
}

void fmc_write_8bit_data(uint32_t address, uint16_t length, int8_t *data_8)
{
    uint32_t i;

    /* unlock the flash program erase controller */
    fmc_unlock();
    /* clear pending flags */
    drv_fmc_flag_clear();

    /* write data_8 to the corresponding address */
    for(i = 0; i < length; i++) {
        if(FMC_READY == fmc_byte_program(address, data_8[i])) {
            address++;
        } else {
            break;
        }
    }
    /* lock the flash program erase controller */
    fmc_lock();
}

void fmc_read_8bit_data(uint32_t address, uint16_t length, int8_t *data_8)
{
    uint32_t i;

    for(i = 0; i < length; i++) {
        data_8[i] = *(__IO int8_t *)(uintptr_t)address;
        address++;
    }
}

int drv_fmc_erase(uint32_t address, int size)
{
    uint32_t erase_addr;
    uint32_t end_addr;
    fmc_sector_info_struct sector_info;

    if(drv_fmc_range_check(address, size) != 0)
        return -1;

    erase_addr = address;
    end_addr = address + (uint32_t)size - 1U;

    fmc_unlock();
    while(erase_addr <= end_addr){
        sector_info = fmc_sector_info_get(erase_addr);
        if(FMC_WRONG_SECTOR_NAME == sector_info.sector_name){
            fmc_lock();
            return -1;
        }

        drv_fmc_flag_clear();
        if(FMC_READY != fmc_sector_erase(sector_info.sector_num)){
            fmc_lock();
            return -1;
        }

        if(sector_info.sector_end_addr >= FMC_END_ADDRESS)
            break;

        erase_addr = sector_info.sector_end_addr + 1U;
    }
    fmc_lock();

    return 0;
}

int drv_fmc_write(uint32_t address, uint8_t *data, int size)
{
    uint32_t i;
    uint32_t write_addr;

    if(!data)
        return -1;

    if(drv_fmc_range_check(address, size) != 0)
        return -1;

    write_addr = address;

    fmc_unlock();
    drv_fmc_flag_clear();
    for(i = 0; i < (uint32_t)size; i++){
        if(FMC_READY != fmc_byte_program(write_addr, data[i])){
            fmc_lock();
            return -1;
        }
        write_addr++;
    }
    fmc_lock();

    return 0;
}

int drv_fmc_read(uint32_t address, uint8_t *data, int size)
{
    if(!data)
        return -1;

    if(drv_fmc_range_check(address, size) != 0)
        return -1;

    memcpy(data, (const void *)(uintptr_t)address, (uint32_t)size);

    return 0;
}


