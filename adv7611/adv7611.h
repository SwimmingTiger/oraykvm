
#ifndef __ADV7611_H__
#define __ADV7611_H__

/*
*  I2C ADDR
*/
#define ADV7611__I2C		0x98
//#define ADV7611__I2C		0x9A

/*
*  I2C MAP ADDR
*/
#define ADV7611_IO          ADV7611__I2C
#define ADV7611_CEC			0x80
#define ADV7611_INFO		0x7c
#define ADV7611_DPLL		0x4c
#define ADV7611_KSV         0x64
#define ADV7611_EDID        0x6c
#define ADV7611_HDMI        0x68
#define ADV7611_CP          0x44

#define 	TRUE			1
#define 	FALSE			0	

/* CMD */
#define CHECKCABLE		0
#define CHECKHDMIINFO   1
#define EN_EXTEDTD      3   //


typedef enum htx_SCAN_MODE_E{
	SCAN_PROGRESSIVE = 0,
	SCAN_INTERLACED,

}SCAN_MODE_E;


typedef enum htx_SAMPLE_RATE_E{
	SAMPLE_RATE_44100 = 0,
	SAMPLE_RATE_48000 = 2,
	SAMPLE_RATE_32000 = 3,
	SAMPLE_RATE_88200 = 8,
	SAMPLE_RATE_768000 = 9,
	SAMPLE_RATE_96000 = 10,
	SAMPLE_RATE_176000 = 12,
	SAMPLE_RATE_192000 = 14,
	
}SAMPLE_RATE_E;



#pragma pack(push)  //
#pragma pack(1)

typedef struct _HDMI_INFO
{
	unsigned int sacnmode;//P  I
	unsigned int width;
	unsigned int height;
	unsigned int fps;
	unsigned int samplerate;  //audio sample rate
	
}HDMIINFO;

#pragma pack(pop)




#endif


