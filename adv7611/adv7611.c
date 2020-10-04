#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/list.h>
#include <asm/delay.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>

#include "gpio_i2c.h"
#include "adv7611.h"

#define hireg_readl(x)		readl(IO_ADDRESS(x))
#define hireg_writel(v,x)	writel(v, IO_ADDRESS(x))

#define Version "v1.9.1"
/*
v1.3   2017-03-13:添加height检测
v1.4 增加两种i2s配置
v1.5 修改EDID为三星
v1.6 修改三星EDID 添加1920*1200 1600*1200 
v1.7 I制式时 检测到的H*2 ,去掉EDID里800*600@75/72
v1.8 修改PRIM_MODE 为HDMI-GR  修复1024*768偏色
v1.9  修复640*480偏色
*/


static unsigned char adv7611_byte_write(unsigned char chip_addr,
										     unsigned char addr     ,
										     unsigned char data     )
{
    gpio_i2c_write(chip_addr,addr,data);
	return 0;
}

static unsigned char adv7611_byte_read(unsigned char chip_addr, unsigned char addr)
{
    return gpio_i2c_read(chip_addr,addr);
}


static void adv7611_byte_write_table(unsigned char chip_addr,

		unsigned char addr, unsigned char *tbl_ptr, unsigned char tbl_cnt)
{
	unsigned char i = 0;
	for(i = 0; i < tbl_cnt; i ++)
	{
		adv7611_byte_write(chip_addr, (addr + i), *(tbl_ptr + i));
	}
}

int adv7611_open(struct inode * inode, struct file * file)
{
	return 0;
}



int adv7611_close(struct inode * inode, struct file * file)
{
	return 0;
}


HDMIINFO adv7441_check_info()
{	
	
	unsigned char  		temp;
	unsigned int 		HTotPix;
	unsigned int 		VTotPix;
	unsigned short    	wcurrTotField1_Hght;
	unsigned short    	wcurrTotField0_Hght;
	unsigned short    	pix_rep;

	unsigned short    	wtemp1;
	unsigned int    	dwtemp2; 
	unsigned short    	wtemp3;
	unsigned short    	wtemp4;
	unsigned int    	dwtemp5;
	unsigned int    	dwTMDS_Freq;
	unsigned short		HFreq;
	unsigned short 		VFreq;

	HDMIINFO hdmiinfo;
	
		temp = adv7611_byte_read(ADV7611_HDMI, 0x0b);
	hdmiinfo.sacnmode = (temp & 0x20) > 0 ? 1 : 0;   //sacnmode

		temp = adv7611_byte_read(ADV7611_HDMI, 0x07);
	hdmiinfo.width = (temp & 0x1F) << 8;
		temp = adv7611_byte_read(ADV7611_HDMI, 0x08);
	hdmiinfo.width += temp;        		//width

	temp = adv7611_byte_read(ADV7611_HDMI, 0x09);
	hdmiinfo.height = (temp & 0x1F) << 8;
		temp = adv7611_byte_read(ADV7611_HDMI, 0x0a);
	hdmiinfo.height += temp;   			//height

	
		temp = adv7611_byte_read(ADV7611_HDMI, 0x39);
	hdmiinfo.samplerate = (temp & 0x0f);


		temp = adv7611_byte_read(ADV7611_HDMI, 0x1E);
	HTotPix = (temp & 0x3F) << 8;
		temp = adv7611_byte_read(ADV7611_HDMI, 0x1F);
	HTotPix += temp;


		temp = adv7611_byte_read(ADV7611_HDMI, 0x26);
	wcurrTotField0_Hght = (temp & 0x3F) << 8;
		temp = adv7611_byte_read(ADV7611_HDMI, 0x27);
	wcurrTotField0_Hght += temp;
	
		temp = adv7611_byte_read(ADV7611_HDMI, 0x28);
	wcurrTotField1_Hght = (temp & 0x3F) << 8;
		temp = adv7611_byte_read(ADV7611_HDMI, 0x29);
	wcurrTotField1_Hght += temp;
	

	if(hdmiinfo.sacnmode)
	{
		VTotPix =( (wcurrTotField1_Hght >> 1) 
		       + (wcurrTotField0_Hght >> 1))>>1;
	}
	else 
	{
		VTotPix = (wcurrTotField0_Hght) >> 1;
	}

	temp = adv7611_byte_read(ADV7611_HDMI, 0x05);
	pix_rep = (temp & 0x0F);
	pix_rep ++;
	
	temp = (adv7611_byte_read(ADV7611_HDMI, 0x51) << 1)|\
		((adv7611_byte_read(ADV7611_HDMI, 0x52) & 0x80) >> 7);
	
	
	temp = adv7611_byte_read(ADV7611_HDMI, 0x06);
	wtemp1 = ((28636360 / 10) * temp) / (27000000 / 10);   //FXTAL=28.63636Mhz
	dwTMDS_Freq = (unsigned int )wtemp1;	


	wtemp1 = HTotPix;	
	/*HSync Frequency in 100Hz*/
	if(wtemp1 != 0) 
	{
		dwtemp2 = (unsigned int ) (((dwTMDS_Freq * 1000000)/ (wtemp1)) / 10);
		HFreq = (unsigned short ) dwtemp2;
		if(pix_rep != 0)
		{
			HFreq = HFreq/pix_rep ;
		}
	}

	wtemp3 = HFreq;										/*VSync Frequency*/
	wtemp4 = VTotPix;
	if(wtemp4 !=0) 
	{
	
		dwtemp5 = (unsigned int) (wtemp3 * 10 / wtemp4 -2 );
		hdmiinfo.fps = (unsigned short ) dwtemp5;

		if(hdmiinfo.sacnmode)
		{
			hdmiinfo.fps = hdmiinfo.fps ; //hdmiinfo.fps = hdmiinfo.fps << 1;
		}
	}
	
	if(hdmiinfo.sacnmode == 1) 
		hdmiinfo.height = hdmiinfo.height * 2;

	return hdmiinfo;
}


long adv7611_ioctl(struct file *file, unsigned int cmd, unsigned long arg)   

{
	unsigned int __user *argp = (unsigned int __user *)arg;
	unsigned int Cable;
	unsigned int val;

	HDMIINFO hdmiinfo;
	switch(cmd)
	{
		case CHECKCABLE:  //检测hdmi线缆插入
		{
			Cable = (adv7611_byte_read(0x98, 0x6f) & 0x01);
			
			if (copy_to_user(argp, &Cable, sizeof(Cable)))    return -1;
			
			break;
		}
		
		case CHECKHDMIINFO:
		{
			hdmiinfo = adv7441_check_info();

			//printk("Check,fps:%d height:%d width:%d sacnmode:%d samplerate:%d\n", \
				hdmiinfo.fps,hdmiinfo.height,hdmiinfo.width,hdmiinfo.sacnmode,hdmiinfo.samplerate);

			if (copy_to_user(argp, &hdmiinfo, sizeof(hdmiinfo)))    return -1;
			
			break;
		}

		case EN_EXTEDTD:
			val = *(unsigned int *)arg;
			switch(val)
			{
				case TRUE:
				{
					adv7611_byte_write(ADV7611_KSV, 0x74, 0x00);//disable edid
					hireg_writel(0x14, 0x201d0050);//off
					mdelay(1000);
					hireg_writel(0x0, 0x201d0050);//on
					
				}
					break;

				case FALSE:
				{
					adv7611_byte_write(ADV7611_KSV, 0x74, 0x03);
					hireg_writel(0x14, 0x201d0050);//off
					mdelay(1000);
					hireg_writel(0x0, 0x201d0050);//hdmi in on
				}

					break;
			}
			break;
		default:
		{
			printk("invalid adv7611 ioctl cmd\n");
			return -1;
			break;
		}
	}
}
/*
* 设置I2S复用，并选择使用adv7611
*/
#if 0
static void set_mux_for_i2s( void )
{
	
	hireg_writel(0x02,0x200F00C0);
	hireg_writel(0x02,0x200F00C4);
	hireg_writel(0x02,0x200F00C8);
	hireg_writel(0x02,0x200F00B0);
	/*Set I2S Mode*/
	hireg_writel(0x0E,0x201200E0);

	printk("i2smux:GPIO2_4,GPIO2_5,GPIO2_6,GPIO2_0 \n");
	printk("Set_I2S_mux0 for 3516A\n");
}
#endif

static void set_mux_for_i2s( void )
{
	
	hireg_writel(0x02,0x200F01AC);
	hireg_writel(0x02,0x200F01BC);
	hireg_writel(0x02,0x200F01C0);
	hireg_writel(0x02,0x200F01C4);
	/*Set I2S Mode*/
	hireg_writel(0x0E,0x201200E0);
	printk("i2smux:GPIO12_0,GPIO12_4,GPIO12_5,GPIO12_6 \n");
	printk("Set_I2S_mux1 for 3516A\n");
}

static void Adv7611_Reset(unsigned char addr)
{
	adv7611_byte_write(addr, 0xFF, 0x80);//I2C reset
	msleep(10);
}


static void Adv7611_init(unsigned char addr)
{
	adv7611_byte_write(addr, 0xF4, 0x80);//CEC
	adv7611_byte_write(addr, 0xF5, 0x7C);//INFOFRAME
	adv7611_byte_write(addr, 0xF8, 0x4C);//DPLL
	adv7611_byte_write(addr, 0xF9, 0x64);//KSV
	adv7611_byte_write(addr, 0xFA, 0x6C);//EDID
	adv7611_byte_write(addr, 0xFB, 0x68);//HDMI
	adv7611_byte_write(addr, 0xFD, 0x44);//CP
	
	adv7611_byte_write(addr, 0x00, 0x08);	  //v1.9
	adv7611_byte_write(addr, 0x01, 0x06);	  //v1.8
	adv7611_byte_write(addr, 0x02, 0xF5);	
	adv7611_byte_write(addr, 0x03, 0x80);	
	adv7611_byte_write(addr, 0x04, 0x62);	
	adv7611_byte_write(addr, 0x05, 0x2C);	
	adv7611_byte_write(addr, 0x06, 0xA6);	
	adv7611_byte_write(addr, 0x0B, 0x44);	
	adv7611_byte_write(addr, 0x0C, 0x42);	
	adv7611_byte_write(addr, 0x14, 0x6A);	
	adv7611_byte_write(addr, 0x15, 0x80);	
	adv7611_byte_write(addr, 0x19, 0x83);	
	adv7611_byte_write(addr, 0x33, 0x40);	
	adv7611_byte_write(ADV7611_HDMI, 0x6C, 0xF1);	
	adv7611_byte_write(addr, 0x20, 0x70);	
		
}

static void Adv7611_HDMI_set(unsigned char addr)
{

	adv7611_byte_write(ADV7611_CP, 0xBA, 0x01);
	adv7611_byte_write(ADV7611_KSV, 0x40, 0x81);

	adv7611_byte_write(addr, 0x9B, 0x03);
	adv7611_byte_write(addr, 0xC1, 0x01);
	adv7611_byte_write(addr, 0xC2, 0x01);
	adv7611_byte_write(addr, 0xC3, 0x01);
	adv7611_byte_write(addr, 0xC4, 0x01);
	adv7611_byte_write(addr, 0xC5, 0x01);
	adv7611_byte_write(addr, 0xC6, 0x01);
	adv7611_byte_write(addr, 0xC7, 0x01);
	adv7611_byte_write(addr, 0xC8, 0x01);
	adv7611_byte_write(addr, 0xC9, 0x01);
	adv7611_byte_write(addr, 0xCA, 0x01);
	adv7611_byte_write(addr, 0xCB, 0x01);
	adv7611_byte_write(addr, 0xCC, 0x01);
	adv7611_byte_write(addr, 0x00, 0x00);
	adv7611_byte_write(addr, 0x83, 0xFE);
	adv7611_byte_write(addr, 0x6F, 0x0C);
	adv7611_byte_write(addr, 0x85, 0x1F);
	adv7611_byte_write(addr, 0x87, 0x70);
	adv7611_byte_write(addr, 0x8D, 0x04);
	adv7611_byte_write(addr, 0x8E, 0x1E);
	adv7611_byte_write(addr, 0x1A, 0x8A);
	adv7611_byte_write(addr, 0x57, 0xDA);
	adv7611_byte_write(addr, 0x58, 0x01);
	adv7611_byte_write(addr, 0x03, 0x10); //auido i2s mode,16bit
	adv7611_byte_write(addr, 0x75, 0x10);
}

static void Adv7611_EDID_Set(unsigned char addr)
{
	printk("EDID SANSUNG CHANGE\n");
	adv7611_byte_write(ADV7611_KSV, 0x77, 0x00); 

	adv7611_byte_write(addr, 0x00, 0x00); // 
	adv7611_byte_write(addr, 0x01, 0xFF); // 
	adv7611_byte_write(addr, 0x02, 0xFF); // 
	adv7611_byte_write(addr, 0x03, 0xFF); // 
	adv7611_byte_write(addr, 0x04, 0xFF); // 
	adv7611_byte_write(addr, 0x05, 0xFF); // 
	adv7611_byte_write(addr, 0x06, 0xFF); // 
	adv7611_byte_write(addr, 0x07, 0x00); // 
	adv7611_byte_write(addr, 0x08, 0x4C); // 
	adv7611_byte_write(addr, 0x09, 0x2D); // 
	adv7611_byte_write(addr, 0x0A, 0x09); // 
	adv7611_byte_write(addr, 0x0B, 0x05); // 
	adv7611_byte_write(addr, 0x0C, 0x01); // 
	adv7611_byte_write(addr, 0x0D, 0x00); // 
	adv7611_byte_write(addr, 0x0E, 0x00); // 
	adv7611_byte_write(addr, 0x0F, 0x00); //
 
	adv7611_byte_write(addr, 0x10, 0x30); // 
	adv7611_byte_write(addr, 0x11, 0x12); // 
	adv7611_byte_write(addr, 0x12, 0x01); // 
	adv7611_byte_write(addr, 0x13, 0x03); // 
	adv7611_byte_write(addr, 0x14, 0x80); // 
	adv7611_byte_write(addr, 0x15, 0x10); // 
	adv7611_byte_write(addr, 0x16, 0x09); // 
	adv7611_byte_write(addr, 0x17, 0x78); // 
	adv7611_byte_write(addr, 0x18, 0x0A); // 
	adv7611_byte_write(addr, 0x19, 0xEE); // 
	adv7611_byte_write(addr, 0x1A, 0x91); // 
	adv7611_byte_write(addr, 0x1B, 0xA3); // 
	adv7611_byte_write(addr, 0x1C, 0x54); // 
	adv7611_byte_write(addr, 0x1D, 0x4C); // 
	adv7611_byte_write(addr, 0x1E, 0x99); // 
	adv7611_byte_write(addr, 0x1F, 0x26); // 

	adv7611_byte_write(addr, 0x20, 0x0F); // 
	adv7611_byte_write(addr, 0x21, 0x50); // 
	adv7611_byte_write(addr, 0x22, 0x54); // 
	adv7611_byte_write(addr, 0x23, 0xBD); // 
	adv7611_byte_write(addr, 0x24, 0x0F); // 
	adv7611_byte_write(addr, 0x25, 0x00); // 
	adv7611_byte_write(addr, 0x26, 0x81); // 
	adv7611_byte_write(addr, 0x27, 0x00); // 
	adv7611_byte_write(addr, 0x28, 0x81); // 
	adv7611_byte_write(addr, 0x29, 0x80); // 
	adv7611_byte_write(addr, 0x2A, 0x95); // 
	adv7611_byte_write(addr, 0x2B, 0x00); // 
	adv7611_byte_write(addr, 0x2C, 0xA9); // 
	adv7611_byte_write(addr, 0x2D, 0x40); // 
	adv7611_byte_write(addr, 0x2E, 0xB3); // 
	adv7611_byte_write(addr, 0x2F, 0x00); // 

	adv7611_byte_write(addr, 0x30, 0xD1); // 
	adv7611_byte_write(addr, 0x31, 0x00); // 
	adv7611_byte_write(addr, 0x32, 0x01); // 
	adv7611_byte_write(addr, 0x33, 0x00); // 
	adv7611_byte_write(addr, 0x34, 0x01); // 
	adv7611_byte_write(addr, 0x35, 0x00); // 
	adv7611_byte_write(addr, 0x36, 0x02); // 
	adv7611_byte_write(addr, 0x37, 0x3A); // 
	adv7611_byte_write(addr, 0x38, 0x80); // 
	adv7611_byte_write(addr, 0x39, 0x18); // 
	adv7611_byte_write(addr, 0x3A, 0x71); // 
	adv7611_byte_write(addr, 0x3B, 0x38); // 
	adv7611_byte_write(addr, 0x3C, 0x2D); // 
	adv7611_byte_write(addr, 0x3D, 0x40); // 
	adv7611_byte_write(addr, 0x3E, 0x58); // 
	adv7611_byte_write(addr, 0x3F, 0x2C); //
 
	adv7611_byte_write(addr, 0x40, 0x45); // 
	adv7611_byte_write(addr, 0x41, 0x00); // 
	adv7611_byte_write(addr, 0x42, 0xA0); // 
	adv7611_byte_write(addr, 0x43, 0x5A); // 
	adv7611_byte_write(addr, 0x44, 0x00); // 
	adv7611_byte_write(addr, 0x45, 0x00); // 
	adv7611_byte_write(addr, 0x46, 0x00); // 
	adv7611_byte_write(addr, 0x47, 0x1E); // 
	adv7611_byte_write(addr, 0x48, 0x66); // 
	adv7611_byte_write(addr, 0x49, 0x21); // 
	adv7611_byte_write(addr, 0x4A, 0x50); // 
	adv7611_byte_write(addr, 0x4B, 0xB0); // 
	adv7611_byte_write(addr, 0x4C, 0x51); // 
	adv7611_byte_write(addr, 0x4D, 0x00); // 
	adv7611_byte_write(addr, 0x4E, 0x1B); // 
	adv7611_byte_write(addr, 0x4F, 0x30); // 
	
	adv7611_byte_write(addr, 0x50, 0x40); // 
	adv7611_byte_write(addr, 0x51, 0x70); // 
	adv7611_byte_write(addr, 0x52, 0x36); // 
	adv7611_byte_write(addr, 0x53, 0x00); // 
	adv7611_byte_write(addr, 0x54, 0xA0); // 
	adv7611_byte_write(addr, 0x55, 0x5A); // 
	adv7611_byte_write(addr, 0x56, 0x00); // 
	adv7611_byte_write(addr, 0x57, 0x00); // 
	adv7611_byte_write(addr, 0x58, 0x00); // 
	adv7611_byte_write(addr, 0x59, 0x1E); // 
	adv7611_byte_write(addr, 0x5A, 0x00); // 
	adv7611_byte_write(addr, 0x5B, 0x00); // 
	adv7611_byte_write(addr, 0x5C, 0x00); // 
	adv7611_byte_write(addr, 0x5D, 0xFD); // 
	adv7611_byte_write(addr, 0x5E, 0x00); // 
	adv7611_byte_write(addr, 0x5F, 0x18); // 
	
	adv7611_byte_write(addr, 0x60, 0x4B); // 
	adv7611_byte_write(addr, 0x61, 0x1A); // 
	adv7611_byte_write(addr, 0x62, 0x51); // 
	adv7611_byte_write(addr, 0x63, 0x17); // 
	adv7611_byte_write(addr, 0x64, 0x00); // 
	adv7611_byte_write(addr, 0x65, 0x0A); // 
	adv7611_byte_write(addr, 0x66, 0x20); // 
	adv7611_byte_write(addr, 0x67, 0x20); // 
	adv7611_byte_write(addr, 0x68, 0x20); // 
	adv7611_byte_write(addr, 0x69, 0x20); // 
	adv7611_byte_write(addr, 0x6A, 0x20); // 
	adv7611_byte_write(addr, 0x6B, 0x20); // 
	adv7611_byte_write(addr, 0x6c, 0x00); // 
	adv7611_byte_write(addr, 0x6D, 0x00); // 
	adv7611_byte_write(addr, 0x6E, 0x00); // 
	adv7611_byte_write(addr, 0x6F, 0xFC); // 
	
	adv7611_byte_write(addr, 0x70, 0x00); // 
	adv7611_byte_write(addr, 0x71, 0x53); // 
	adv7611_byte_write(addr, 0x72, 0x41); // 
	adv7611_byte_write(addr, 0x73, 0x4D); // 
	adv7611_byte_write(addr, 0x74, 0x53); // 
	adv7611_byte_write(addr, 0x75, 0x55); // 
	adv7611_byte_write(addr, 0x76, 0x4E); // 
	adv7611_byte_write(addr, 0x77, 0x47); // 
	adv7611_byte_write(addr, 0x78, 0x0A); // 
	adv7611_byte_write(addr, 0x79, 0x20); // 
	adv7611_byte_write(addr, 0x7A, 0x20); // 
	adv7611_byte_write(addr, 0x7B, 0x20); // 
	adv7611_byte_write(addr, 0x7C, 0x20); // 
	adv7611_byte_write(addr, 0x7D, 0x20); // 
	adv7611_byte_write(addr, 0x7E, 0x01); // 
	adv7611_byte_write(addr, 0x7F, 0x3A); // 
	
	adv7611_byte_write(addr, 0x80, 0x02); // 
	adv7611_byte_write(addr, 0x81, 0x03); // 
	adv7611_byte_write(addr, 0x82, 0x27); // 
	adv7611_byte_write(addr, 0x83, 0xF1); // 
	adv7611_byte_write(addr, 0x84, 0x4B); // 
	adv7611_byte_write(addr, 0x85, 0x90); // 
	adv7611_byte_write(addr, 0x86, 0x1F); // 
	adv7611_byte_write(addr, 0x87, 0x04); // 
	adv7611_byte_write(addr, 0x88, 0x13); // 
	adv7611_byte_write(addr, 0x89, 0x05); // 
	adv7611_byte_write(addr, 0x8A, 0x14); // 
	adv7611_byte_write(addr, 0x8B, 0x03); // 
	adv7611_byte_write(addr, 0x8C, 0x12); // 
	adv7611_byte_write(addr, 0x8D, 0x20); // 
	adv7611_byte_write(addr, 0x8E, 0x21); // 
	adv7611_byte_write(addr, 0x8F, 0x22); // 
	
	adv7611_byte_write(addr, 0x90, 0x23); // 
	adv7611_byte_write(addr, 0x91, 0x09); // 
	adv7611_byte_write(addr, 0x92, 0x07); // 
	adv7611_byte_write(addr, 0x93, 0x07); // 
	adv7611_byte_write(addr, 0x94, 0x83); // 
	adv7611_byte_write(addr, 0x95, 0x01); // 
	adv7611_byte_write(addr, 0x96, 0x00); // 
	adv7611_byte_write(addr, 0x97, 0x00); // 
	adv7611_byte_write(addr, 0x98, 0xE2); // 
	adv7611_byte_write(addr, 0x99, 0x00); // 
	adv7611_byte_write(addr, 0x9A, 0x0F); // 
	adv7611_byte_write(addr, 0x9B, 0xE3); // 
	adv7611_byte_write(addr, 0x9C, 0x05); // 
	adv7611_byte_write(addr, 0x9D, 0x03); // 
	adv7611_byte_write(addr, 0x9E, 0x01); // 
	adv7611_byte_write(addr, 0x9F, 0x67); // 
	
	adv7611_byte_write(addr, 0xA0, 0x03); // 
	adv7611_byte_write(addr, 0xA1, 0x0C); // 
	adv7611_byte_write(addr, 0xA2, 0x00); // 
	adv7611_byte_write(addr, 0xA3, 0x40); // 
	adv7611_byte_write(addr, 0xA4, 0x00); // 
	adv7611_byte_write(addr, 0xA5, 0xB8); // 
	adv7611_byte_write(addr, 0xA6, 0x2D); // 
	adv7611_byte_write(addr, 0xA7, 0x01); // 
	adv7611_byte_write(addr, 0xA8, 0x1D); // 
	adv7611_byte_write(addr, 0xA9, 0x00); // 
	adv7611_byte_write(addr, 0xAA, 0x72); // 
	adv7611_byte_write(addr, 0xAB, 0x51); // 
	adv7611_byte_write(addr, 0xAC, 0xD0); // 
	adv7611_byte_write(addr, 0xAD, 0x1E); // 
	adv7611_byte_write(addr, 0xAE, 0x20); // 
	adv7611_byte_write(addr, 0xAF, 0x6E); // 
	
	adv7611_byte_write(addr, 0xB0, 0x28); // 
	adv7611_byte_write(addr, 0xB1, 0x55); // 
	adv7611_byte_write(addr, 0xB2, 0x00); // 
	adv7611_byte_write(addr, 0xB3, 0xA0); // 
	adv7611_byte_write(addr, 0xB4, 0x5A); // 
	adv7611_byte_write(addr, 0xB5, 0x00); // 
	adv7611_byte_write(addr, 0xB6, 0x00); // 
	adv7611_byte_write(addr, 0xB7, 0x00); // 
	adv7611_byte_write(addr, 0xB8, 0x1E); // 
	adv7611_byte_write(addr, 0xB9, 0x01); // 
	adv7611_byte_write(addr, 0xBA, 0x1D); // 
	adv7611_byte_write(addr, 0xBB, 0x00); // 
	adv7611_byte_write(addr, 0xBC, 0xBC); // 
	adv7611_byte_write(addr, 0xBD, 0x52); // 
	adv7611_byte_write(addr, 0xBE, 0xD0); // 
	adv7611_byte_write(addr, 0xBF, 0x1E); // 
	
	adv7611_byte_write(addr, 0xC0, 0x20); // 
	adv7611_byte_write(addr, 0xC1, 0xB8); // 
	adv7611_byte_write(addr, 0xC2, 0x28); // 
	adv7611_byte_write(addr, 0xC3, 0x55); // 
	adv7611_byte_write(addr, 0xC4, 0x40); // 
	adv7611_byte_write(addr, 0xC5, 0xA0); // 
	adv7611_byte_write(addr, 0xC6, 0x5A); // 
	adv7611_byte_write(addr, 0xC7, 0x00); // 
	adv7611_byte_write(addr, 0xC8, 0x00); // 
	adv7611_byte_write(addr, 0xC9, 0x00); // 
	adv7611_byte_write(addr, 0xCA, 0x1E); // 
	adv7611_byte_write(addr, 0xCB, 0x01); // 
	adv7611_byte_write(addr, 0xCC, 0x1D); // 
	adv7611_byte_write(addr, 0xCD, 0x80); // 
	adv7611_byte_write(addr, 0xCE, 0x18); // 
	adv7611_byte_write(addr, 0xCF, 0x71); // 
	
	adv7611_byte_write(addr, 0xD0, 0x1C); // 
	adv7611_byte_write(addr, 0xD1, 0x16); // 
	adv7611_byte_write(addr, 0xD2, 0x20); // 
	adv7611_byte_write(addr, 0xD3, 0x58); // 
	adv7611_byte_write(addr, 0xD4, 0x2C); // 
	adv7611_byte_write(addr, 0xD5, 0x25); // 
	adv7611_byte_write(addr, 0xD6, 0x00); // 
	adv7611_byte_write(addr, 0xD7, 0xA0); // 
	adv7611_byte_write(addr, 0xD8, 0x5A); // 
	adv7611_byte_write(addr, 0xD9, 0x00); // 
	adv7611_byte_write(addr, 0xDA, 0x00); // 
	adv7611_byte_write(addr, 0xDB, 0x00); // 
	adv7611_byte_write(addr, 0xDC, 0x9E); // 
	adv7611_byte_write(addr, 0xDD, 0x01); // 
	adv7611_byte_write(addr, 0xDE, 0x1D); // 
	adv7611_byte_write(addr, 0xDF, 0x80); // 
	
	adv7611_byte_write(addr, 0xE0, 0xD0); // 
	adv7611_byte_write(addr, 0xE1, 0x72); // 
	adv7611_byte_write(addr, 0xE2, 0x1C); // 
	adv7611_byte_write(addr, 0xE3, 0x16); // 
	adv7611_byte_write(addr, 0xE4, 0x20); // 
	adv7611_byte_write(addr, 0xE5, 0x10); // 
	adv7611_byte_write(addr, 0xE6, 0x2C); // 
	adv7611_byte_write(addr, 0xE7, 0x25); // 
	adv7611_byte_write(addr, 0xE8, 0x80); // 
	adv7611_byte_write(addr, 0xE9, 0xA0); // 
	adv7611_byte_write(addr, 0xEA, 0x5A); // 
	adv7611_byte_write(addr, 0xEB, 0x00); // 
	adv7611_byte_write(addr, 0xEC, 0x00); // 
	adv7611_byte_write(addr, 0xED, 0x00); // 
	adv7611_byte_write(addr, 0xEE, 0x9E); // 
	adv7611_byte_write(addr, 0xEF, 0x00); // 

	adv7611_byte_write(addr, 0xF0, 0x00); // 
	adv7611_byte_write(addr, 0xF1, 0x00); // 
	adv7611_byte_write(addr, 0xF2, 0x00); // 
	adv7611_byte_write(addr, 0xF3, 0x00); // 
	adv7611_byte_write(addr, 0xF4, 0x00); // 
	adv7611_byte_write(addr, 0xF5, 0x00); // 
	adv7611_byte_write(addr, 0xF6, 0x00); // 
	adv7611_byte_write(addr, 0xF7, 0x00); // 
	adv7611_byte_write(addr, 0xF8, 0x00); // 
	adv7611_byte_write(addr, 0xF9, 0x00); // 
	adv7611_byte_write(addr, 0xFA, 0x00); // 
	adv7611_byte_write(addr, 0xFB, 0x00); // 
	adv7611_byte_write(addr, 0xFC, 0x00); // 
	adv7611_byte_write(addr, 0xFD, 0x00); // 
	adv7611_byte_write(addr, 0xFE, 0x00); // 
	adv7611_byte_write(addr, 0xFF, 0xED); // 
	
	adv7611_byte_write(ADV7611_KSV, 0x77, 0x00); // Set the Most Significant Bit of the SPA location to 0
	adv7611_byte_write(ADV7611_KSV, 0x52, 0x20); // Set the SPA for port B.
	adv7611_byte_write(ADV7611_KSV, 0x53, 0x00); // Set the SPA for port B.
	adv7611_byte_write(ADV7611_KSV, 0x70, 0x9E); // Set the Least Significant Byte of the SPA location
#if 0
	adv7611_byte_write(ADV7611_KSV, 0x74, 0x03); // Enable the Internal EDID for Ports
#else
	adv7611_byte_write(ADV7611_KSV, 0x74, 0x00); // Enable the Internal EDID for Ports
#endif
	adv7611_byte_write(ADV7611_IO, 0x20, 0xF0); //HPA MANUAL

}


static struct file_operations adv7611_fops = 
{
	.owner      = THIS_MODULE,
	.unlocked_ioctl  = adv7611_ioctl, 
	.open       = adv7611_open,
	.release    = adv7611_close
};



static struct miscdevice adv7611_dev = 
{
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= "adv7611",
	.fops  		= &adv7611_fops,
};



static int __init adv7611_module_init(void)	
{
	int ret = 0;
	unsigned char reg;

	printk("ADV7611 EDID Version:%s\n",Version);
	/* register misc device*/
	ret = misc_register(&adv7611_dev);
	if (ret)
	{
		printk("ERROR: could not register adv7611 devices\n");
		return ret;
	}
	
	ret = adv7611_byte_write(ADV7611_IO, 0xF4, 0x80);
	if(ret != 0)
	{
		printk("ERROR : write adv7611 register fail \n");
		return -1;
	}

	reg = adv7611_byte_read(ADV7611_IO,0xF4);
	if(reg != 0x80)
	{
		printk("ERROR : read adv7611 register is 0x%02x\n",reg);
		return -1;
	}
	else 
		printk("Now adv7611 driver init successful!\n");

	set_mux_for_i2s();
	Adv7611_Reset(ADV7611_IO);
	Adv7611_init(ADV7611_IO);
	Adv7611_HDMI_set(ADV7611_HDMI);
	Adv7611_EDID_Set(ADV7611_EDID);


	return 0;
}



static void __exit adv7611_module_exit(void)

{
	misc_deregister(&adv7611_dev);
}


module_init(adv7611_module_init);

module_exit(adv7611_module_exit);



#ifdef MODULE

//#include <linux/compile.h>

#endif

MODULE_LICENSE("GPL");
