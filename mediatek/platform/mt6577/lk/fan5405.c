#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/i2c.h>     
#include <platform/fan5405.h>
#include <printf.h>

/**********************************************************
  *
  *   [I2C Slave Setting] 
  *
  *********************************************************/
#define fan5405_SLAVE_ADDR_WRITE 0xD4
#define fan5405_SLAVE_ADDR_Read	0xD5

/**********************************************************
  *
  *   [Global Variable] 
  *
  *********************************************************/
#define fan5405_REG_NUM 7  
kal_uint8 fan5405_reg[fan5405_REG_NUM] = {0};

/**********************************************************
  *
  *   [I2C Function For Read/Write fan5405] 
  *
  *********************************************************/
U32 fan5405_i2c_read (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;

    ret_code = i2c_v1_write(I2C0, chip, cmdBuffer, cmdBufferLen);    // set register command
    if (ret_code != I2C_OK)
        return ret_code;

    ret_code = i2c_v1_read(I2C0, chip, dataBuffer, dataBufferLen);

    //printf("[fan5405_i2c_read] Done\n");

    return ret_code;
}

U32 fan5405_i2c_write (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;
    U8 write_data[I2C_FIFO_SIZE];
    int transfer_len = cmdBufferLen + dataBufferLen;
    int i=0, cmdIndex=0, dataIndex=0;

    if(I2C_FIFO_SIZE < (cmdBufferLen + dataBufferLen))
    {
        printf("[fan5405_i2c_write] exceed I2C FIFO length!! \n");
        return 0;
    }

    //write_data[0] = cmd;
    //write_data[1] = writeData;

    while(cmdIndex < cmdBufferLen)
    {
        write_data[i] = cmdBuffer[cmdIndex];
        cmdIndex++;
        i++;
    }

    while(dataIndex < dataBufferLen)
    {
        write_data[i] = dataBuffer[dataIndex];
        dataIndex++;
        i++;
    }

    /* dump write_data for check */
    for( i=0 ; i < transfer_len ; i++ )
    {
        //printf("[fan5405_i2c_write] write_data[%d]=%x\n", i, write_data[i]);
    }

    ret_code = i2c_v1_write(I2C0, chip, write_data, transfer_len);

    //printf("[fan5405_i2c_write] Done\n");

    return ret_code;
}

/**********************************************************
  *
  *   [Read / Write Function] 
  *
  *********************************************************/
kal_uint32 fan5405_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    U8 chip_slave_address = fan5405_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    U32 result_tmp;

	printf("--------------------------------------------------\n");

    cmd = RegNum;
    result_tmp = fan5405_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    
	printf("[fan5405_read_interface] Reg[%x]=0x%x\n", RegNum, data);
	
	data &= (MASK << SHIFT);
	*val = (data >> SHIFT);
	
	printf("[fan5405_read_interface] val=0x%x\n", *val);

	return 1;
}

kal_uint32 fan5405_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    U8 chip_slave_address = fan5405_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    U32 result_tmp;

	printf("--------------------------------------------------\n");

    cmd = RegNum;
    result_tmp = fan5405_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    printf("[fan5405_config_interface] Reg[%x]=0x%x\n", RegNum, data);

    data &= ~(MASK << SHIFT);
    data |= (val << SHIFT);

    result_tmp = fan5405_i2c_write(chip_slave_address, &cmd, cmd_len, &data, data_len);
    printf("[fan5405_config_interface] write Reg[%x]=0x%x\n", RegNum, data);

    // Check
    result_tmp = fan5405_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    printf("[fan5405_config_interface] Check Reg[%x]=0x%x\n", RegNum, data);
    
	return 1;
}

/**********************************************************
  *
  *   [fan5405 Function] 
  *
  *********************************************************/
//CON0
void fan5405_set_tmr_rst(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON0), 
									(kal_uint8)(val),
									(kal_uint8)(CON0_TMR_RST_MASK),
									(kal_uint8)(CON0_TMR_RST_SHIFT)
									);
}

unsigned int fan5405_get_slrst_status(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=fan5405_read_interface( 	(kal_uint8)(fan5405_CON0), 
									(&val),
									(kal_uint8)(CON0_SLRST_MASK),
									(kal_uint8)(CON0_SLRST_SHIFT)
									);
	return val;
}

void fan5405_set_en_stat(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON0), 
									(kal_uint8)(val),
									(kal_uint8)(CON0_EN_STAT_MASK),
									(kal_uint8)(CON0_EN_STAT_SHIFT)
									);
}

unsigned int fan5405_get_chip_status(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=fan5405_read_interface( 	(kal_uint8)(fan5405_CON0), 
									(&val),
									(kal_uint8)(CON0_STAT_MASK),
									(kal_uint8)(CON0_STAT_SHIFT)
									);
	return val;
}

unsigned int fan5405_get_fault_reason(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=fan5405_read_interface( 	(kal_uint8)(fan5405_CON0), 
									(&val),
									(kal_uint8)(CON0_FAULT_MASK),
									(kal_uint8)(CON0_FAULT_SHIFT)
									);
	return val;
}

//CON1
void fan5405_set_lin_limit(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON1), 
									(kal_uint8)(val),
									(kal_uint8)(CON1_LIN_LIMIT_MASK),
									(kal_uint8)(CON1_LIN_LIMIT_SHIFT)
									);
}

void fan5405_set_lowv_2(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON1), 
									(kal_uint8)(val),
									(kal_uint8)(CON1_LOW_V_2_MASK),
									(kal_uint8)(CON1_LOW_V_2_SHIFT)
									);
}

void fan5405_set_lowv_1(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON1), 
									(kal_uint8)(val),
									(kal_uint8)(CON1_LOW_V_1_MASK),
									(kal_uint8)(CON1_LOW_V_1_SHIFT)
									);
}

void fan5405_set_te(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON1), 
									(kal_uint8)(val),
									(kal_uint8)(CON1_TE_MASK),
									(kal_uint8)(CON1_TE_SHIFT)
									);
}

void fan5405_set_ce(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON1), 
									(kal_uint8)(val),
									(kal_uint8)(CON1_CE_MASK),
									(kal_uint8)(CON1_CE_SHIFT)
									);
}

void fan5405_set_hz_mode(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON1), 
									(kal_uint8)(val),
									(kal_uint8)(CON1_HZ_MODE_MASK),
									(kal_uint8)(CON1_HZ_MODE_SHIFT)
									);
}

void fan5405_set_opa_mode(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON1), 
									(kal_uint8)(val),
									(kal_uint8)(CON1_OPA_MODE_MASK),
									(kal_uint8)(CON1_OPA_MODE_SHIFT)
									);
}

//CON2
void fan5405_set_cv_vth(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON2), 
									(kal_uint8)(val),
									(kal_uint8)(CON2_CV_VTH_MASK),
									(kal_uint8)(CON2_CV_VTH_SHIFT)
									);
}

//CON3
unsigned int fan5405_get_vender_code(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=fan5405_read_interface( 	(kal_uint8)(fan5405_CON3), 
									(&val),
									(kal_uint8)(CON3_VENDER_CODE_MASK),
									(kal_uint8)(CON3_VENDER_CODE_SHIFT)
									);
	return val;
}

unsigned int fan5405_get_pin(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=fan5405_read_interface( 	(kal_uint8)(fan5405_CON3), 
									(&val),
									(kal_uint8)(CON3_PIN_MASK),
									(kal_uint8)(CON3_PIN_SHIFT)
									);
	return val;
}

unsigned int fan5405_get_revision(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=fan5405_read_interface( 	(kal_uint8)(fan5405_CON3), 
									(&val),
									(kal_uint8)(CON3_REVISION_MASK),
									(kal_uint8)(CON3_REVISION_SHIFT)
									);
	return val;
}

//CON4
void fan5405_set_reset(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON4), 
									(kal_uint8)(val),
									(kal_uint8)(CON4_RESET_MASK),
									(kal_uint8)(CON4_RESET_SHIFT)
									);
}

void fan5405_set_ac_charging_current(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON4), 
									(kal_uint8)(val),
									(kal_uint8)(CON4_I_CHR_MASK),
									(kal_uint8)(CON4_I_CHR_SHIFT)
									);
}

void fan5405_set_termination_current(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON4), 
									(kal_uint8)(val),
									(kal_uint8)(CON4_I_TERM_MASK),
									(kal_uint8)(CON4_I_TERM_SHIFT)
									);
}

//CON5
void fan5405_set_low_chg(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON5), 
									(kal_uint8)(val),
									(kal_uint8)(CON5_LOW_CHG_MASK),
									(kal_uint8)(CON5_LOW_CHG_SHIFT)
									);
}

unsigned int fan5405_get_dpm_status(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=fan5405_read_interface( 	(kal_uint8)(fan5405_CON5), 
									(&val),
									(kal_uint8)(CON5_DPM_STATUS_MASK),
									(kal_uint8)(CON5_DPM_STATUS_SHIFT)
									);
	return val;
}

unsigned int fan5405_get_cd_status(void)
{
	kal_uint32 ret=0;
	kal_uint8 val=0;

	ret=fan5405_read_interface( 	(kal_uint8)(fan5405_CON5), 
									(&val),
									(kal_uint8)(CON5_CD_STATUS_MASK),
									(kal_uint8)(CON5_CD_STATUS_SHIFT)
									);
	return val;
}

void fan5405_set_vsreg(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON5), 
									(kal_uint8)(val),
									(kal_uint8)(CON5_VSREG_MASK),
									(kal_uint8)(CON5_VSREG_SHIFT)
									);
}

//CON6
void fan5405_set_mchrg(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON6), 
									(kal_uint8)(val),
									(kal_uint8)(CON6_MCHRG_MASK),
									(kal_uint8)(CON6_MCHRG_SHIFT)
									);
}

void fan5405_set_mreg(unsigned int val)
{
	kal_uint32 ret=0;	

	ret=fan5405_config_interface(	(kal_uint8)(fan5405_CON6), 
									(kal_uint8)(val),
									(kal_uint8)(CON6_MREG_MASK),
									(kal_uint8)(CON6_MREG_SHIFT)
									);
}

/**********************************************************
  *
  *   [Internal Function] 
  *
  *********************************************************/
//debug liao
unsigned int fan5405_config_interface_liao (unsigned char RegNum, unsigned char val)
{
    kal_uint32 ret=0;
	kal_uint8 ret_val=0;

    printf("--------------------------------------------------\n");
    
    //fan5405_read_byte(RegNum, &fan5405_reg);
	//ret=fan5405_read_interface(RegNum,&val,0xFF,0x0);
    //printk("[liao fan5405_config_interface] Reg[%x]=0x%x\n", RegNum, val);
    
    //fan5405_write_byte(RegNum, val);
    ret=fan5405_config_interface(RegNum,val,0xFF,0x0);
    printf("[liao fan5405_config_interface] write Reg[%x]=0x%x\n", RegNum, val);

    // Check
    //fan5405_read_byte(RegNum, &fan5405_reg);
    ret=fan5405_read_interface(RegNum,&ret_val,0xFF,0x0);
    printf("[liao fan5405_config_interface] Check Reg[%x]=0x%x\n", RegNum, ret_val);

    return 1;
}

void fan5405_dump_register(void)
{
    U8 chip_slave_address = fan5405_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    int i=0;
    for (i=0;i<fan5405_REG_NUM;i++)
	{
        cmd = i;
        fan5405_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
        fan5405_reg[i] = data;
        printf("[fan5405_dump_register] Reg[0x%X]=0x%X\n", i, fan5405_reg[i]);
    }
}

void fan5405_read_register(int i)
{
    U8 chip_slave_address = fan5405_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;

    cmd = i;
    fan5405_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    fan5405_reg[i] = data;
    printf("[fan5405_read_register] Reg[0x%X]=0x%X\n", i, fan5405_reg[i]);
}

void fan5405_hw_init(void)
{	
#if defined(HIGH_BATTERY_VOLTAGE_SUPPORT)
    printf("[fan5405_hw_init] (0x06,0x77)\n");
    fan5405_config_interface_liao(0x06,0x77); // set ISAFE and HW CV point (4.34)
#else
    printf("[fan5405_hw_init] (0x06,0x70)\n");
    fan5405_config_interface_liao(0x06,0x70); // set ISAFE
#endif
}

