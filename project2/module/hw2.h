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

// major number 지정
#define MAJOR_NUMBER 242

// device driver 및 각각 device의 physical address 지정
#define HW2_DEVICE_DRIVER "dev_driver"
#define FND_ADDRESS 0x08000004
#define LED_ADDRESS 0x08000016
#define LCD_ADDRESS 0x08000090
#define DOT_ADDRESS 0x08000210

// cmd값을 지정
#define SET_OPTION 0
#define COMMAND 1

// usage count 지정
static int usage_count = 0;

// physical address를 mapping시킬 포인터
static unsigned char *led_addr;
static unsigned char *lcd_addr;
static unsigned char *fnd_addr;
static unsigned char *dot_addr;

// ioctl을 통해 user program에서 받아오는 structure
static struct hw2_info{
	unsigned char t_interval;
	unsigned char t_cnt;
	int t_init;
	unsigned char st_name[32];
};

// timer 변수
static struct hw2_timer{
	struct timer_list timer;
	int count;
};

//fpga dot를 위한 값
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

//module를 위한 변수들
struct hw2_info info;
struct hw2_timer my_timer;
// fpga_led 값을 미리 저장해둠.
const unsigned char led_light[9] = {0, 128, 64, 32, 16, 8, 4, 2, 1};
// led_idx는 led의 D[1~8] idx를 나타내려고 한다
int led_idx;
// fnd_idx는 숫자가 출력되어야하는 fnd index의 값이다.
int fnd_idx;
// 아래 lcd_flag는 각각 위와 아래줄의 text가 왼쪽으로 이동할지 오른쪽으로 이동할지 정해주는 flag
int lcd_flag1, lcd_flag2;
// lcd_idx은 lcd의 위 아래줄의 가장 왼쪽 글자의 idx값이다.
int lcd_idx1, lcd_idx2;
// move_count는 숫자가 1바퀴 돌았는지 count해주는 역할을 한다.
int move_count = 8;
unsigned char fnd_light[4];
unsigned char lcd_light[33];

// timer를 종료할 때 초기화 시켜주는 함수이다.
void ft_init(void);
// interval이 지났을 때마다 출력되어야 할 숫자와 lcd text를 옮겨주는 함수이다.
void ft_move(void);
// fnd에 입력될 숫자를 fnd_light에 저장시킨다.
void ft_cal_fnd(void);
// led_idx와 fnd_idx 값을 구하여 저장시킨다.
void ft_cal_led(void);
// fpga에 출력해주는 함수이다.
void ft_print_driver(void);
// timer를 종료할 때 초기화 후 print함수를 호출해주는 함수이다.
void ft_close_driver(void);
// timer의 function부분을 작성하였다.
static void ft_hw2_timer(unsigned long);
// ioctl에서 command가 들어왔을 떄 수행된다.
void hw2_command(void);
//fops로 재정의한 함수들
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
