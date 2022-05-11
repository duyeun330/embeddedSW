#ifndef _HW2_H_
#define _HW2_H_

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>


#define MAJOR_NUMBER 242

#define HW2_DEVICE_DRIVER "dev_driver"
#define FND_ADDRESS 0x08000004
#define LED_ADDRESS 0x08000016
#define LCD_ADDRESS 0x08000090
#define DOT_ADDRESS 0x08000210

#define SET_OPTION 0
#define COMMAND 1

static int usage_count = 0;

static unsigned char *led_addr;
static unsigned char *lcd_addr;
static unsigned char *fnd_addr;
static unsigned char *dot_addr;

static struct hw2_info{
	unsigned char t_interval;
	unsigned char t_cnt;
	int t_init;
	unsigned char st_name[32];
};

static struct hw2_timer{
	struct timer_list timer;
	int count;
};

unsigned char fpga_number[10][10] = {
     {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // blank 
     {0x0c,0x1c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e}, // 1
     {0x7e,0x7f,0x03,0x03,0x3f,0x7e,0x60,0x60,0x7f,0x7f}, // 2
     {0xfe,0x7f,0x03,0x03,0x7f,0x7f,0x03,0x03,0x7f,0x7e}, // 3
     {0x66,0x66,0x66,0x66,0x66,0x66,0x7f,0x7f,0x06,0x06}, // 4
	{0x7f,0x7f,0x60,0x60,0x7e,0x7f,0x03,0x03,0x7f,0x7e}, // 5
	{0x60,0x60,0x60,0x60,0x7e,0x7f,0x63,0x63,0x7f,0x3e}, // 6
	{0x7f,0x7f,0x63,0x63,0x03,0x03,0x03,0x03,0x03,0x03}, // 7
	{0x3e,0x7f,0x63,0x63,0x7f,0x7f,0x63,0x63,0x7f,0x3e}, // 8
	{0x3e,0x7f,0x63,0x63,0x7f,0x3f,0x03,0x03,0x03,0x03} // 9
};

struct hw2_info info;
struct hw2_timer my_timer;
const unsigned char led_light[9] = {0, 128, 64, 32, 16, 8, 4, 2, 1};
int led_idx;
int fnd_idx;
int lcd_flag1, lcd_flag2;
int lcd_idx1, lcd_idx2;
int move_count = 8;
unsigned char fnd_light[4];
unsigned char lcd_light[33];

void ft_init(void);
void ft_move(void);
void ft_cal_fnd(void);
void ft_cal_led(void);
void ft_print_driver(void);
void ft_close_driver(void);
static void ft_hw2_timer(unsigned long);
void hw2_command(void);
int hw2_open(struct inode *minode, struct file *mfile);
int hw2_release(struct inode *minode, struct file *mfile);
static long hw2_driver_ioctl(struct file *, unsigned int, unsigned long);

struct file_operations hw2_fops =
{
	.owner			=	THIS_MODULE,
	.open			=	hw2_open,
	.release			=	hw2_release,
	.unlocked_ioctl	=	hw2_driver_ioctl,
};

#endif
