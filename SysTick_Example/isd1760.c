/***********************************************************************************************************************
 * Name:            ISD1760.C
 * 
 * Description      ����Ĩ ISD1760 �� ����ϱ� ���� ���α׷� �Լ���
 *
 * Date:            2015-06-05
 *
 * Author:          ���� 
 **********************************************************************************************************************/

#ifndef     _ISD1760_C_
#define     _ISD1760_C_
             
#include    <isd1760.h>

unsigned char g_volume = 15;      // SJM 190716 added by SJM. originally it was 15(constant)

unsigned char spi(unsigned char data)
{

  unsigned char rValue;
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1, data);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  rValue = SPI_I2S_ReceiveData(SPI1);    
  return rValue;
}

/***********************************************************************************************************************
 * ISD1760�� �ʱ�ȭ �Ѵ�
 *
 * @author      ����
 * @date        2015-06-04
 **********************************************************************************************************************/
void ISD1760_init (void)
{
    //
    // SPI �� �ʱ�ȭ �Ѵ�
    // SPI Enable; Data Order: LSB first; SCK: high when idle; Setup: rising edge of SCK
    //
    //SPCR = 0x7C; 

        SPI_InitTypeDef  SPI_InitStructure;
        GPIO_InitTypeDef GPIO_InitStructure;
        
        //Uart_printf1("\rLTC2492_init\n");

	//SPI2 Periph clock enable 
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
        //Configure SPI1 pins: SCK, MISO and MOSI 
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
        
        /* SPI SCK pin configuration */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        
        /* SPI  MISO pin configuration */
        GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
        GPIO_Init(GPIOA, &GPIO_InitStructure);  
        
        /* SPI  MOSI pin configuration */
        GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        //Configure PA8 : ISD1760 CS pin 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        /* SPI configuration -------------------------------------------------------*/
        SPI_I2S_DeInit(SPI1);
        
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	
        
        /* Configure the Priority Group to 1 bit */                
        //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
        
        /* Configure the SPI interrupt priority */
        /*NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);*/
        
        SPI_Init(SPI1, &SPI_InitStructure);
        
	SPI_Cmd(SPI1, ENABLE);
        //SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);

        
        
	return;	
}

/***********************************************************************************************************************
 * ISD1760 �� ���ؼ� PowerUp �� �����Ѵ�
 *
 * @author      ����
 * @return      contents of SR0 register. See ISD1700 datasheet for more information
 * @date        2015-06-04
 **********************************************************************************************************************/
unsigned short ISD1760_powerUp (void)
{
    unsigned short rValue = 0;

    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    
    rValue = spi(0x11);
    rValue <<= 8;
    rValue |= spi(0x00);

    Delay(1);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);

    return rValue;
}

/***********************************************************************************************************************
 * ISD1760 �� ���ؼ� PowerDown �� �����Ѵ�
 *
 * @author      ����
 * @return      contents of SR0 register. See ISD1700 datasheet for more information 
 * @date        2009-01-28
 **********************************************************************************************************************/
unsigned short ISD1760_powerDown (void)
{
    unsigned short rValue = 0;
    unsigned long status;
    unsigned short count;

    // Stop ������
    count = 0;
    do
    {
        ISD1760_stop();
        status = ISD1760_readStatus();
        count++;
    } while((status & CHECK_CMDERR) && count < ISD1760_TIMEOUT);
    //Uart_printf1("\r\nstop count = %d", count);
        
    // Interrupt ��ٸ���
    count = 0;
    do
    {
        status = ISD1760_readStatus();
        count++;
    } while(!(status & CHECK_INT) && count < 10);
    //Uart_printf1("\r\nint count = %d", count);

    // ClearINT ����
    count = 0;
    do
    {
        ISD1760_clearInt();
        status = ISD1760_readStatus();
        count++;
    } while ((status & CHECK_CMDERR) && count < ISD1760_TIMEOUT);
    //Uart_printf1("clear count = %d\n", count);
     
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    rValue = spi(COMMAND_PD);
    rValue <<= 8;
    rValue |= spi(0x00);
    Delay(1);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);

    return rValue;
}

/***********************************************************************************************************************
 * ISD1760 �� ���ؼ� STOP�� �����Ѵ�. �ϰ� �ִ� ������ �ߴ��Ѵ�
 *
 * @author      ����
 * @return      contents of SR0 register. See ISD1700 datasheet for more information
 * @date        2009-01-28
 **********************************************************************************************************************/
unsigned short ISD1760_stop (void)
{
    unsigned short rValue = 0;
    
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    rValue = spi(COMMAND_STOP);
    rValue <<= 8;
    rValue |= spi(0x00);
    Delay(1);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
   
    return rValue;
}

/***********************************************************************************************************************
 * ISD1760 �� ���ؼ� ���ͷ�Ʈ�� �����Ѵ�. �̷��� �����ν� ���� ����� ������ �� �ִ�
 *
 * @author      ����
 * @return      contents of SR0 register. See ISD1700 datasheet for more information
 * @date        2009-01-28
 **********************************************************************************************************************/
unsigned short ISD1760_clearInt (void)
{
    unsigned long rValue = 0;
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    
    rValue = spi(COMMAND_CLR_INT);
    rValue <<= 8;
    rValue |= spi(0x00);

    Delay(1);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);

    return rValue;
}

/***********************************************************************************************************************
 * ISD1760 �� ���ؼ� ���¸� �д´�. ISD1760 �� �� ����� �޾Ƶ��� �غ� �Ǿ��ִ��� Ȯ���Ѵ�
 *
 * @author      ����
 * @return      contents of SR0 register and SR1 register. See ISD1700 datasheet for more information
 * @date        2009-01-28
 **********************************************************************************************************************/
unsigned long ISD1760_readStatus (void)
{
    unsigned long rValue = 0;

    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    
    rValue = spi(COMMAND_RD_STATUS);

    rValue <<= 8;
    rValue |= spi(0x00);
    rValue <<=8;
    rValue |= spi(0x00);
    
    Delay(5);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
    
    //printf("rValue : %x\r\n", rValue);
    return rValue;
}

/***********************************************************************************************************************
 * ISD1760 �� ���ؼ� Reset �� �����Ѵ�. ���� � ��ɾ ����ǰ� �ִٸ� ���߰� Power Down ���·� ����
 *
 * @author      ����
 * @return      contents of SR0 register. See ISD1700 datasheet for more information
 * @date        2009-01-28
 **********************************************************************************************************************/
unsigned short ISD1760_reset (void)
{
    unsigned short rValue = 0;

    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    rValue = spi(COMMAND_RESET);
    rValue <<= 8;
    rValue |= spi(0);
    Delay(1);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);

    return rValue;
}

/***********************************************************************************************************************
 * ISD1760 �� ���ؼ� RD_APC �� �����Ѵ�.
 *
 * @author      ����
 * @return      contents of SR0 and APC register
 * @date        2009-01-28
 **********************************************************************************************************************/
unsigned long ISD1760_readAPC (void)
{
    unsigned long rValue = 0;

    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    rValue = spi(COMMAND_RD_APC);
    rValue <<= 8;
    rValue |= spi(0x00);
    rValue <<= 8;
    rValue |= spi(0x00);
    rValue <<= 8;
    rValue |= spi(0x00);
    Delay(1);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);

    return rValue;
}

/***********************************************************************************************************************
 * ISD1760 �� ���ؼ� WR_NVCFG �� �����Ѵ�.
 *
 * @author      ����
 * @return      contents of SR0
 * @date        2009-01-28
 **********************************************************************************************************************/
unsigned short ISD1760_writeNVCFG (void)
{
    unsigned short rValue = 0;

    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    rValue = spi(COMMAND_WR_NVCFG);
    rValue <<= 8;
    rValue |= spi(0x00);
    Delay(1);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);

    return rValue;
}
#if 0     // SJM 190625 NoUse
/***********************************************************************************************************************
 * ISD1760 �� ���ؼ� LD_NVCFG �� �����Ѵ�.
 *
 * @author      ����
 * @return      contents of SR0
 * @date        2009-01-28
 **********************************************************************************************************************/
unsigned short ISD1760_loadNVCFG  (void)
{
    unsigned short rValue = 0;

    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    rValue = spi(COMMAND_LD_NVCFG);
    rValue <<= 8;
    rValue |= spi(0x00);
    Delay(1);    
    GPIO_SetBits(GPIOA, GPIO_Pin_8);

    return rValue;
}
#endif    // if 0 SJM 190625 NoUse
/***********************************************************************************************************************
 * ISD1760 �� ���ؼ� WR_APC2 �� �����Ѵ�. �ַ� Volume control �� ���ؼ� ���δ�
 *
 * @author      ����
 * @param       data, [in] data to write on APC register
 * @return      contents of SR0 register and 1st byte of SR0. See ISD1700 datasheet for more information
 * @date        2009-01-28
 **********************************************************************************************************************/
unsigned long ISD1760_writeAPC2 (unsigned short data)
{
    unsigned long rValue = 0;

    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    rValue = spi(COMMAND_WR_APC2);
    rValue <<= 8;
    rValue |= spi(data & 0xFF);
    rValue <<= 8;
    rValue |= spi(data >> 8);
    Delay(1);    
    GPIO_SetBits(GPIOA, GPIO_Pin_8);

    return rValue;
}
#if 0     // SJM 190625 NoUse
/***********************************************************************************************************************
 * ISD1760 �� ���ؼ� G_ERASE �� �����Ѵ�.
 *
 * @author      ����
 * @return      contents of SR0
 * @date        2009-01-28
 **********************************************************************************************************************/
unsigned short ISD1760_globalErase (void)
{
    unsigned short rValue = 0;

    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    rValue = spi(COMMAND_G_ERASE);
    rValue <<= 8;
    rValue |= spi(0x00);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);

    return rValue;
}

/***********************************************************************************************************************
 * ������ �����ּҿ� �� �ּҿ� ���ؼ� ������ �����Ѵ�. 
 * �׻� setErase �� ó���� ȣ���ؾ� �Ѵ�
 * ����
 * 1. ISD1760_setErase �� ȣ���Ѵ�
 * 2. SetRec �� �����ϰ� ����� �������� Ȯ���Ѵ�
 * 3. Power Down �� �����Ѵ�
 *
 * @author      ����
 * @param       startAddr, [in] start address
 * @param       endAddr, [in] end address
 * @date        2009-01-28
 **********************************************************************************************************************/
void ISD1760_setRecord (unsigned short startAddr, unsigned short endAddr)
{
    unsigned long status;

    // Reset ����
    do
    {
        status = ISD1760_reset();
        status = ISD1760_readStatus();
    } while(status & CHECK_PU); 

    // Power up �� �ϰ� CMD_ERR Ȯ��
    //printf("\r\nSetRecord : PowerUp");
    do
    {
        ISD1760_powerUp();
        Delay(DELAY_TPUD);
        status = ISD1760_readStatus();
    } while(!(status & CHECK_PU || status & CMD_ERR));
    
    // Clear Int �����ϰ� CHECK_RDY Ȯ��
    //printf("\r\nSetRecord : ClearInt()");
    status = ISD1760_clearInt();
    do
    {
        status = ISD1760_readStatus();
    } while(!(status & CHECK_RDY));
           
///////////////////////////////////////////
    // Write APC2
    //printf("\r\nSetRecord : APC2()");    
    ISD1760_writeAPC2(0x400);
    do
    {
        status = ISD1760_readStatus();
    } while(!(status & CHECK_RDY));
    
    //printf("\r\nSetRecord : NVCFG()");    
    ISD1760_writeNVCFG();
    do
    {
        status = ISD1760_readStatus();
    } while(!(status & CHECK_RDY));
    
    // Clear Int �����ϰ� CHECK_RDY Ȯ��
    //printf("\r\nSetRecord : ClearInt()");    
    status = ISD1760_clearInt();
    do
    {
        status = ISD1760_readStatus();
    } while(!(status & CHECK_RDY));
////////////////////////////////////////////
    //printf("\r\nSetRecord : SET_REC ...");    
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    spi(COMMAND_SET_REC);
    spi(0x00);
    spi(startAddr & 0xFF);
    spi(startAddr >> 8);
    spi(endAddr & 0xFF);
    spi(endAddr >> 8);
    spi(0x00);
    Delay(1);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
                      
    //mdelay(3);
    //status = ISD1760_readStatus();
    
    // ��ɾ �������� Ȯ��
    //printf("\r\nSetRecord : Wait INT()");    
    do
    {
        status = ISD1760_readStatus();
    } while(!(status & CHECK_INT));
    
    // Clear Int �����ϰ� CHECK_RDY Ȯ��
    //printf("\r\nSetRecord : ClearInt()");       
    status = ISD1760_clearInt();    
    do
    {
        status = ISD1760_readStatus();
    } while(!(status & CHECK_RDY));
    
    //printf("\r\nSetRecord : PowerDown()");   
    do
    {
        status = ISD1760_powerDown();
    } while(status & CHECK_PU);
}
#endif    // if 0 SJM 190625 NoUse

/***********************************************************************************************************************
 * ������ �����ּҿ� �� �ּҿ� ���ؼ� ����� �����Ѵ�
 * ����
 * 1. Reset�� �����Ѵ�. ������ Play �� �־��ٸ� �����
 * 2. Power Up�� �ϰ� CMD_ERR �� Ȯ���Ѵ� �׸��� Tpud ��ŭ ��ٸ���
 * 3. Clear Int �� ������ �� CHECK_RDY �� Ȯ���Ѵ�
 * 4. Set Play �� �����Ѵ�
 * �� �Լ��� Play �� �� �������� Ȯ������ �ʴ´�
 *
 * @author      ����
 * @param       startAddr, [in] start address
 * @param       endAddr, [in] end address
 * @date        2009-06-05
 * @history
 *  2009-06-05  ���ѷ��� ������ ��� ����
 **********************************************************************************************************************/
void ISD1760_setPlay (unsigned short startAddr, unsigned short endAddr, unsigned char volume)
{
    unsigned long status;
    unsigned short count;
    
    // ���� �̹� Play ���̶�� Stop �Ѵ�
    status = ISD1760_readStatus();

    if(status & CHECK_PLAYING)
    {
        // Stop ������
        count = 0;
        do
        {
            ISD1760_stop();
            status = ISD1760_readStatus();
            count++;
        } while((status & CHECK_CMDERR) && count < ISD1760_TIMEOUT );
        
        // Interrupt ��ٸ���
        count = 0;
        do
        {
            status = ISD1760_readStatus();
            count++;
        } while(!(status & CHECK_INT) && count < ISD1760_INT_TIMEOUT);
        
        // ClearINT ����
        count = 0;
        do
        {
            ISD1760_clearInt();
            status = ISD1760_readStatus();
            count++;
        } while ((status & CHECK_CMDERR) && count < ISD1760_TIMEOUT);
    }
    
    // ���� PU �� �ƴ϶�� Power up ����
    count = 0;
    ISD1760_clearInt();
    do
    {
        status = ISD1760_readStatus();
        count++;
    } while(!(status & CHECK_RDY) && count < ISD1760_TIMEOUT);

    
    // ���� ������ �������� �ʴٸ� Power up ����
    if(!(status & CHECK_PU))
    {
        count = 0;
        do
        {
            ISD1760_powerUp();
            Delay(DELAY_TPUD);
            status = ISD1760_readStatus();
            count++;
        
            // Timeout
            if(count >= ISD1760_TPUD_TIMEOUT) break;
            
        } while( !(status & CHECK_PU) || (status & CMD_ERR) ); 
    }
    
    // Set Volume
    ISD1760_setVolume(volume);
    //ISD1760_writeAPC2(volume);
    // RDY bit ����
    count = 0;
    ISD1760_clearInt();
    do
    {
        status = ISD1760_readStatus();
        count++;
    } while(!(status & CHECK_RDY) && count < ISD1760_TIMEOUT);
    

    // Set Play
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    spi(COMMAND_SET_PLAY);
    spi(0x00);
    spi(startAddr & 0xFF);
    spi(startAddr >> 8);
    spi(endAddr & 0xFF);
    spi(endAddr >> 8);
    spi(0x00);
    Delay(1);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
    Delay(3);
    status = ISD1760_readStatus();
}
#if 0     // SJM 190625 NoUse
/***********************************************************************************************************************
 * ������ �����ּҿ� �� �ּҿ� ���ؼ� ������ �����Ѵ�
 * ����
 * 1. Power Up ���� �� CMD_ERR Ȯ���ϰ� Tpud ���� ��ٸ���
 * 2. Clear Int �����ϰ� CHECK_RDY �Ѵ�
 * 3. SetErase �ϰ� CHECK_INT �ؼ� ��ɾ �������� Ȯ���Ѵ�.
 * 4. Clear Int �����ϰ� CHECK_RDY �Ѵ�.
 * ���� �� ����� �����ϸ� �׻� ISD1760 �� Power up ���¿��� ���ο� ����� �޾Ƶ��� �غ� �ȴ�
 *
 * @author      ����
 * @param       startAddr, [in] start address
 * @param       endAddr, [in] end address
 * @date        2009-01-23
 **********************************************************************************************************************/
void ISD1760_setErase (unsigned short startAddr, unsigned short endAddr)
{
    unsigned long status;

    // Reset ����
    do
    {
        status = ISD1760_reset();
        status = ISD1760_readStatus();
    } while(status & CHECK_PU); 
    
    // Power up �� �ϰ� CMD_ERR Ȯ��
    do
    {
      
        ISD1760_powerUp();
        Delay(DELAY_TPUD);
        status = ISD1760_readStatus();
    } while(!(status & CHECK_PU || status & CMD_ERR));
    
    // Clear Int �����ϰ� CHECK_RDY Ȯ��
    status = ISD1760_clearInt();
    do
    {
      
        status = ISD1760_readStatus();
    } while(!(status & CHECK_RDY));

    // Set erase
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    spi(COMMAND_SET_ERASE);
    spi(0x00);
    spi(startAddr & 0xFF);
    spi(startAddr >> 8);
    spi(endAddr & 0xFF);
    spi(endAddr >> 8);
    spi(0x00);
    Delay(1);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
                 
    Delay(1);
    status = ISD1760_readStatus();

    // ��ɾ �������� Ȯ��
    do
    {
      
        status = ISD1760_readStatus();
    } while(!(status & CHECK_INT));
    
    // Clear Int �����ϰ� CHECK_RDY Ȯ��
    status = ISD1760_clearInt();    
    do
    {
      
        status = ISD1760_readStatus();
    } while(!(status & CHECK_RDY));
    
    do
    {
      
        status = ISD1760_powerDown();
    } while(status & CHECK_PU);
}
#endif    // if 0 SJM 190625 NoUse

/***********************************************************************************************************************
 * Volume �� �����Ѵ�
 * ����
 * 1. Power down �� �ؼ� ���� ����� ����� �� �� �ִٸ� �Ϸ��ϰ� �����ϵ��� �Ѵ�
 * 2. Power up �� �ؼ� Idle ���·� �����. �Ŀ� Tpud ��ŭ ��ٸ���
 * 3. Clear Int �� �����Ѵ�.
 * 4. WR_APC2 ����� �����Ѵ�. �Ϸ��ϸ� CHECK_RDY �� �ؼ� ��ɾ ������ �غ���°� �ǵ��� �Ѵ�
 *
 * @author      ����
 * @param       volume, [in] ���ϴ� volume
 * @date        2009-01-28
 **********************************************************************************************************************/
void ISD1760_setVolume (unsigned char volume)
{
    unsigned long status;
    unsigned short count;

    //printf("v1\r\n");
    // Check RDY bit?
    count = 0;
    ISD1760_clearInt();
    do
    {
        status = ISD1760_readStatus();
        count++;
    } while(!(status & CHECK_RDY) && count < ISD1760_TIMEOUT_COUNTER);
    //printf("v2\r\n");
    // ���� ������ �������� �ʴٸ� Power up ����
    if(!(status & CHECK_PU))
    {
        count = 0;
        do
        {
            ISD1760_powerUp();
            Delay(DELAY_TPUD);
            status = ISD1760_readStatus();
            count++;
            //printf("status : 0x%x\r\n", status);
            // Timeout
            if(count >= ISD1760_TIMEOUT_COUNTER) break;
            
        } while( !(status & CHECK_PU) || (status & CMD_ERR) );
    }
    //printf("v3\r\n");
    status = ISD1760_readAPC();
    status = status & 0xFFFF;
    //printf("v4\r\n");
    // Check volume setting
    status >>= 8;
    if((status & 0x07) != (7 - volume))
    {
        // Write APC2
        count = 0;
        do
        {
            status = ISD1760_readStatus();
            count++;
        } while(!(status & CHECK_RDY) && count < ISD1760_TIMEOUT_COUNTER);
        
        ISD1760_writeAPC2(DEFAULT_APC | ((7 - volume) & 0x07));
        //printf("\r\n calc : %d", DEFAULT_APC | ((7 - volume) & 0x07));
        count = 0;
        do
        {
            status = ISD1760_readStatus();
            count++;
        } while(!(status & CHECK_RDY) && count < ISD1760_TIMEOUT_COUNTER);
        
        ISD1760_writeNVCFG();
        count = 0;
        do
        {
            status = ISD1760_readStatus();
            count++;
        } while(!(status & CHECK_RDY) && count < ISD1760_TIMEOUT_COUNTER);
    }
    //printf("v5\r\n");
}
#if 0     // SJM 190625 NoUse
/***********************************************************************************************************************
 * Test Program
 **********************************************************************************************************************/
void TestISD1760 (void)
{
    unsigned long status;
    unsigned short startAddr = 0x10, endAddr = 0x2F;
    //printf("\r\nTest Start");    
    do
    {
        status = ISD1760_powerUp();
        Delay(DELAY_TPUD);
        status = ISD1760_readStatus();
    } while(!(status & CHECK_PU) || status & CMD_ERR);

    //printf("\r\nPower up End!!");

    ISD1760_clearInt();
    do
    {
        status = ISD1760_readStatus();
    } while(!(status & CHECK_RDY));

    //printf("\r\nSet erase Start!!");    
    // Set erase
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    spi(COMMAND_SET_ERASE);
    spi(0x00);
    spi(startAddr & 0xFF);
    spi(startAddr >> 8);
    spi(endAddr & 0xFF);
    spi(endAddr >> 8);
    spi(0x00);
    Delay(1);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
    
    //printf("\r\nSet erase End!!");

    do
    {
        status = ISD1760_readStatus();
    } while(!(status & CHECK_INT));
                 
    //printf("\r\nSet Clear Start!!");

    ISD1760_clearInt();
    //printf("\r\nSet Clear End!!");

    do
    {
        status = ISD1760_readStatus();
    } while(!(status & CHECK_RDY));

    //printf("\r\nSet set Record!!");
             
    // Set Record
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    spi(COMMAND_SET_REC);
    spi(0x00);
    spi(startAddr & 0xFF);
    spi(startAddr >> 8);
    spi(endAddr & 0xFF);
    spi(endAddr >> 8);
    spi(0x00);
    Delay(1);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
    
    do {
        status = ISD1760_readStatus();
    } while(!(status & CHECK_INT));
    
    ISD1760_clearInt();
    do
    {
        status = ISD1760_readStatus();
    } while(!(status & CHECK_RDY));
    
    //ISD1760_writeAPC2(10);
    //printf("\r\nSet Play!!");
    // Play
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    spi(COMMAND_SET_PLAY);
    spi(0x00);
    spi(startAddr & 0xFF);
    spi(startAddr >> 8);
    spi(endAddr & 0xFF);
    spi(endAddr >> 8);
    spi(0x00);
    Delay(1);
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
    
    do
    {
        status = ISD1760_readStatus();
    } while(!(status & CHECK_INT));
    
    // Power down �� �ϰ� CMD_ERR Ȯ��. ���� ��� ����� ���� �ִٸ� �Ϸ��ϰ� Power �ٿ� ���·� ����
    //printf("\r\nPower Down!!");
    do
    {
        status = ISD1760_powerDown();
    } while(status & CMD_ERR);
    
    //printf("\r\nFunction End!!!");
}

/***********************************************************************************************************************
 * Analog Path �� �����Ѵ�
 *
 * @author      ����
 * @date        2009-06-05
 **********************************************************************************************************************/
void ISD1760_setAnalogPath (void)
{
    unsigned long status;
    unsigned short count;
    //printf("\r\nISD1760_setAnalogPath Start!!!");
    // Check RDY bit?
    count = 0;
    ISD1760_clearInt();
    do
    {
        status = ISD1760_readStatus();
        count++;
    } while(!(status & CHECK_RDY) && count < ISD1760_TIMEOUT_COUNTER);
    
    // ���� ������ �������� �ʴٸ� Power up ����
    if(!(status & CHECK_PU))
    {
        count = 0;
        do
        {
            ISD1760_powerUp();
            Delay(DELAY_TPUD);
            status = ISD1760_readStatus();
            count++;
        
            // Timeout
            if(count >= ISD1760_TIMEOUT_COUNTER) break;
            
        } while( !(status & CHECK_PU) || (status & CMD_ERR) );
    }
    
    status = ISD1760_readAPC();
    status = status & 0xFFFF;
    if(status != 0x4004)
    {
        // Write APC2
        ISD1760_writeAPC2(0x440);
        count = 0;
        do
        {
            status = ISD1760_readStatus();
            count++;
        } while(!(status & CHECK_RDY) && count < ISD1760_TIMEOUT_COUNTER);
        
        ISD1760_writeNVCFG();
        count = 0;
        do
        {
            status = ISD1760_readStatus();
            count++;
        } while(!(status & CHECK_RDY) && count < ISD1760_TIMEOUT_COUNTER);
    }
    
   //printf("\r\nISD1760_setAnalogPath End!!!");
}
#endif // if 0 SJM 190625 NoUse
#endif      /* _ISD1760_C_ */