#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

#define MAJOR_NUM 242
#define FND_ADDRESS 0x08000004

// timer structure
struct struct_mydata {
	struct timer_list timer;
};

static int result;
static dev_t inter_dev;
static int	inter_major = 0;
static struct cdev inter_cdev;
static struct struct_mydata mytimer;
static struct struct_mydata mytimer1;
static unsigned char *fnd_addr;

// fops로 정의할 함수들
static int inter_open(struct inode *, struct file *);
static int inter_release(struct inode *, struct file *);
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

// 사용할 각각의 handler 함수들
irqreturn_t inter_home(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_back(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_volup(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_voldown(int irq, void* dev_id, struct pt_regs* reg);

// usage count
static int inter_usage=0;
// 출력을 위한 버퍼
unsigned char fnd_light[4];
// fnd 출력값을 계산하기 위한 변수들
static int fnd;
static int home_b, paused;
static unsigned long paused_time;

// bottom half 함수 구현
void	bottom_half_func(void);
DECLARE_TASKLET(bottom_half, bottom_half_func, 0);

wait_queue_head_t	wq_write;
DECLARE_WAIT_QUEUE_HEAD(wq_write);

static struct file_operations inter_fops =
{
	.open = inter_open,
	.write = inter_write,
	.release = inter_release,
};

// 출력되어야할 fnd값을 저장 후 출력
void	print_fnd(void){
	unsigned short int	p_fnd;
	int	tmp1, tmp2;
	// 초와 분으로 나누어 계산 후 출력해준다
	tmp1 = fnd / 60;
	tmp2 = fnd % 60;
	fnd_light[0] = tmp1 / 10;
	fnd_light[1] = tmp1 % 10;
	fnd_light[2] = tmp2 / 10;
	fnd_light[3] = tmp2 % 10;

	p_fnd = fnd_light[0] << 12 | fnd_light[1] << 8 | fnd_light[2] << 4 | fnd_light[3];
	outw(p_fnd, (unsigned int)fnd_addr);
}
// fnd값 초기화
void	init_fnd(void){
	fnd = 0;
	outw(0, (unsigned int)fnd_addr);
}
// start 할 때 부르는 timer function
static void	kernel_timer_blink(unsigned long timeout) {
	fnd++;	// fnd 값을 1초에 1씩 증가
	print_fnd();
	
	mytimer.timer.expires = get_jiffies_64() + HZ;
	mytimer.timer.data = (unsigned long)&mytimer;
	mytimer.timer.function = kernel_timer_blink;
	add_timer(&mytimer.timer);
}

// bottom half function 구현, 이를 제외한 함수는 top half
void	bottom_half_func(){
	__wake_up(&wq_write, 1, 1, NULL);
	printk(KERN_ALERT "WAKE UP PROCESS\n");
}

// 종료시 실행되는 함수로 모든 timer을 제거하고 fnd 초기화, bottom half함수를 콜하여 wake up
static void	end_func(unsigned long timeout){
	printk(KERN_ALERT "Stopwatch End\n");
	del_timer(&(mytimer1.timer));
	del_timer(&(mytimer.timer));
	init_fnd();
	tasklet_schedule(&bottom_half);
}
// home button을 눌렀을 때 함수
irqreturn_t inter_home(int irq, void* dev_id, struct pt_regs* reg) {
	printk(KERN_ALERT "HOME BUTTON = %x\n", gpio_get_value(IMX_GPIO_NR(1, 11)));
	if (!home_b && paused) {	// puase되어있을 때 재실행할 때 전의 값을 더하여 다시 stopwatch 실행
		home_b = 1;
		mytimer.timer.expires = get_jiffies_64() + paused_time;
		mytimer.timer.data = (unsigned long)&mytimer;
		mytimer.timer.function = kernel_timer_blink;
		add_timer(&(mytimer.timer));
		paused_time = 0;
		paused = 0;
	}
	else if (!home_b) {	// pause가 아니라 처음실행할 때
		home_b = 1;
		mytimer.timer.expires = get_jiffies_64() + HZ;
		mytimer.timer.data = (unsigned long)&mytimer;
		mytimer.timer.function = kernel_timer_blink;
		add_timer(&(mytimer.timer));

	}
	return IRQ_HANDLED;
}

//	back button을 눌렀을 떄 handler로 전의 값을 0.1초까지 저장
irqreturn_t inter_back(int irq, void* dev_id, struct pt_regs* reg) {
	printk(KERN_ALERT "BACK BUTTON = %x\n", gpio_get_value(IMX_GPIO_NR(1, 12)));
	if (home_b) {
		paused_time = mytimer.timer.expires - get_jiffies_64();
		paused_time = (paused_time / 10) * 10;	// 다음과 같이 하여 HZ값이 0.01초까지 나타내므로 유효숫자가 0.1초로 된다.
		home_b = 0;
		paused = 1;
		del_timer(&(mytimer.timer));
	}
	return IRQ_HANDLED;
}

// volume up button을 눌렀을 때 실행되는 함수로 모든 값을 reset 한다.
irqreturn_t inter_volup(int irq, void* dev_id,struct pt_regs* reg) {
	printk(KERN_ALERT "VOLUME UP BUTTON = %x\n", gpio_get_value(IMX_GPIO_NR(2, 15)));
	del_timer(&(mytimer.timer));
	init_fnd();
	paused_time = 0;
	paused = 0;
	if (home_b) {
		mytimer.timer.expires = get_jiffies_64() + HZ;
		mytimer.timer.data = (unsigned long)&mytimer;
		mytimer.timer.function = kernel_timer_blink;
		add_timer(&(mytimer.timer));
	}
	return IRQ_HANDLED;
}

// volume down button을 눌렀을 떄 실행되는 handler
irqreturn_t inter_voldown(int irq, void* dev_id, struct pt_regs* reg) {
	unsigned int	val = gpio_get_value(IMX_GPIO_NR(5, 14));
	printk(KERN_ALERT "VOLUME DOWN = %x\n", val);
     if (val) {	//val == 1이라는 것은 release되었을 때 라는 뜻
		del_timer(&(mytimer1.timer));
	}
	else {	// 눌렸다면 mytimer1을 3초로 등록하여 end_func을 function으로 등록
		mytimer1.timer.expires = get_jiffies_64() + 3 * HZ;
		mytimer1.timer.data = (unsigned long)&mytimer1;
		mytimer1.timer.function = end_func;
		add_timer(&(mytimer1.timer));
	}
	return IRQ_HANDLED;
}

// app에서 open함수를 불렀을 때 불려지는 함수
static int inter_open(struct inode *minode, struct file *mfile){
	int ret;
	int irq;
	
	if (inter_usage != 0)
		return -EBUSY;
	inter_usage = 1;
	printk(KERN_ALERT "Open Module\n");

	// home button
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_home, IRQF_TRIGGER_FALLING, "home", 0);

	// back button
	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_back, IRQF_TRIGGER_FALLING, "back", 0);

	// vol +
	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_volup, IRQF_TRIGGER_FALLING, "volup", 0);

	// vol -
	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq = gpio_to_irq(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_voldown, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "voldown", 0);

	return 0;
}

static int inter_release(struct inode *minode, struct file *mfile){
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);
	inter_usage = 0;	
	printk(KERN_ALERT "Release Module\n");
	return 0;
}

// write함수를 불렀을 때 불리는 함수
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos ){
     printk("sleep on\n");
	init_fnd();
	home_b = paused = 0;
	paused_time = 0;
	printk("write function, sleep on\n");
	interruptible_sleep_on(&wq_write);
	return 0;
}

// device를 register하는 함수
static int inter_register_cdev(void)
{
	int error;

	inter_dev = MKDEV(MAJOR_NUM, 0);
	error = register_chrdev_region(inter_dev,1,"stopwatch");
	if(error<0) {
		printk(KERN_WARNING "stopwatch: can't get major %d\n", inter_major);
		return result;
	}
	printk(KERN_ALERT "major number = %d\n", MAJOR_NUM);
	cdev_init(&inter_cdev, &inter_fops);
	inter_cdev.owner = THIS_MODULE;
	inter_cdev.ops = &inter_fops;
	error = cdev_add(&inter_cdev, inter_dev, 1);
	if(error)
	{
		printk(KERN_NOTICE "stopwatch Register Error %d\n", error);
	}
	return 0;
}

static int __init inter_init(void) {
	int result;

	if((result = inter_register_cdev()) < 0 )
		return result;
	fnd_addr = ioremap(FND_ADDRESS, 0x4);
	init_timer(&(mytimer.timer));
	init_timer(&(mytimer1.timer));
	printk(KERN_ALERT "Init Module Success \n");
	printk(KERN_ALERT "Device : /dev/stopwatch, Major Num : 242 \n");
	return 0;
}

static void __exit inter_exit(void) {
	cdev_del(&inter_cdev);
	unregister_chrdev_region(inter_dev, 1);
	iounmap(fnd_addr);
	del_timer_sync(&(mytimer.timer));
	del_timer_sync(&(mytimer1.timer));
	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(inter_init);
module_exit(inter_exit);
	MODULE_LICENSE("GPL");
