#include "hw2.h"

void ft_init(){
	int i;
	
	// module 내에서 쓰이는 변수들을 초기화시킨다.
	led_idx = fnd_idx = lcd_flag1 = lcd_flag2 = 0;
	lcd_idx1 = lcd_idx2 = 0;
	move_count = 8;
	for (i = 0; i < 32; i++)
		lcd_light[i] = info.st_name[i];
}

void ft_move(){
	int i, j;
	int tmp = 1;
	
	// led_idx의 값을 1~8까지 바꿔주는 부분
	if (led_idx == 8)
		led_idx = 1;
	else 
		led_idx++;

	// move_count가 끝나면 fnd index를 한칸 옮겨준다.
	if (move_count == 0) {
		move_count = 8;
		fnd_idx = (fnd_idx + 1) % 4;
	}
	
	// 출력되어야하는 값을 init에 계산 후 저장
	for (i = 0; i < 3 - fnd_idx; i++)
		tmp *= 10;
	tmp *= led_idx;
	info.t_init = tmp;
	
	//lcd text의 위치를 저장해주는 부분
	// 4개의 if문은 위줄의 text가 움직여야 하는 방향을 체크해준다
	if (lcd_flag1 && lcd_idx1 == 0) lcd_flag1 = 0;
	if (!lcd_flag1 && lcd_idx1 == 8) lcd_flag1 = 1;
	if (lcd_flag2 && lcd_idx2 == 0) lcd_flag2 = 0;
	if (!lcd_flag2 && lcd_idx2 == 3) lcd_flag2 = 1;
	
	// lcd_flag1 == 1이면 위의 text가 왼쪽으로 한칸 움직여야 한다.
	if (lcd_flag1) {
		j = 0;
		for (i = lcd_idx1; j < 8; j++) {
			lcd_light[i - 1] = lcd_light[i];
			i++;
		}
		lcd_light[i - 1] = ' ';
		lcd_idx1--;
	}
	else { // lcd_flag1 == 0 이면 오른쪽으로 한칸 움직인다.
		j = 0;
		for (i = lcd_idx1 + 8; j < 8; j++) {
			lcd_light[i] = lcd_light[i - 1];
			i--;
		}
		lcd_light[i] = ' ';
		lcd_idx1++;
	}

	// lcd_flag2는 lcd 의 아래 text의 움직이는 방향이다.
	if (lcd_flag2) {
		j = 0;
		for (i = lcd_idx2 + 16; j < 13; j++) {
			lcd_light[i - 1] = lcd_light[i];
			i++;
		}
		lcd_light[i - 1] = ' ';
		lcd_idx2--;
	}
	else {
		j = 0;
		for (i = lcd_idx2 + 29; j < 13; j++) {
			lcd_light[i] = lcd_light[i - 1];
			i--;
		}
		lcd_light[i] = ' ';
		lcd_idx2++;
	}
}

// fnd light에 알맞은 값을 저장
void ft_cal_fnd(){
	int tmp = info.t_init;
	int tmp2;
	int i;

	for (i = 3 ; i >= 0 ; i--) {
		tmp2 = tmp % 10;
		tmp /= 10;
		fnd_light[i] = (unsigned char)tmp2;
	}
}

// 처음에 led_idx와 fnd_idx 값을 구해주는 함수
void ft_cal_led(){
	int tmp = info.t_init;
	int tmp2;
	int i;

	for (i = 3; i >= 0; i--){
		tmp2 = tmp % 10;
		if (tmp2){
			led_idx = tmp2;
			fnd_idx = i;
			break ;
		}
		tmp /= 10;
	}
}

// 각각의 디바이스로 출력해주는 함수
void ft_print_driver(){
	unsigned short p_led;
	unsigned short int p_fnd;
	unsigned short int p_dot;
	unsigned short int p_lcd;
	int i;

	// led 출력
	p_led = (unsigned short)led_light[led_idx];
	outw(p_led, (unsigned int)led_addr);
	
	// fnd 출력
	p_fnd = fnd_light[0] << 12 | fnd_light[1] << 8 | fnd_light[2] << 4 | fnd_light[3];
	printk("print fnd %d led %d\n", (int)p_fnd, led_light[led_idx]);
	outw(p_fnd, (unsigned int)fnd_addr);
	
	// dot 출력
	for (i = 0; i < 10; i++){
		p_dot = fpga_number[led_idx][i] & 0x7F;
		outw(p_dot, (unsigned int)dot_addr + i * 2);
	}

	// lcd 출력
	for (i = 0; i < 32; i+=2){
		p_lcd = (lcd_light[i] & 0xFF) << 8 | (lcd_light[i + 1] & 0xFF);
		outw(p_lcd, (unsigned int)lcd_addr + i);
	}
}

// 닫을 때 호출되어 드라이버 출력을 초기화 시켜준다.
void ft_close_driver(){
	int i;
	for (i = 0; i < 4; i++)
		fnd_light[i] = 0;
	led_idx = 0;
	for (i = 0; i < 33; i++)
		lcd_light[i] = 0;
	ft_print_driver();
}

static void ft_hw2_timer(unsigned long timeout) {
	struct hw2_timer *t = (struct hw2_timer *)timeout;	
	// 움직일 때 move_count를 하나 줄인다.
	// 총 count값을 하나 줄이고 디바이스로 출력한다.
	move_count--;
	ft_move();
	t->count--;
	ft_cal_fnd();
	ft_print_driver();
	printk("time %d\n", t->count);

	// 모두 출력했다면 close한다.
	if (t->count == 0){
		ft_close_driver();
		return ;
	}
	my_timer.timer.expires = get_jiffies_64() + (1 * info.t_interval * HZ / 10);
	my_timer.timer.data = (unsigned long)&my_timer;
	my_timer.timer.function = ft_hw2_timer;
	add_timer(&my_timer.timer);
}

// command가 호출되었을 때 실행되는 함수
void hw2_command(){
	my_timer.count = info.t_cnt;
	my_timer.timer.expires = get_jiffies_64() + (1 * info.t_interval * HZ / 10);
	my_timer.timer.data = (unsigned long)&my_timer;
	my_timer.timer.function = ft_hw2_timer;
	add_timer(&my_timer.timer);

}

// ioctl 함수를 작성
static long hw2_driver_ioctl(struct file *mfile, unsigned int cmd, unsigned long args)
{

	printk("ioctl() call\n");
	switch (cmd) {
		case SET_OPTION:	// set_option일 때 초기화 한 후 처음 화면 출력
			if (copy_from_user(&info, (void __user *)args, sizeof(info)))
				return -EFAULT;
			ft_init();
			ft_cal_fnd();
			ft_cal_led();
			ft_print_driver();
			break ;

		case COMMAND:	// command함수 수행
			hw2_command();
			break ;
		default:
			printk("wrong\n");
			break ;
	}
	return (0);
}

// open release에 usage count를 증가 감소 시킨다.
int hw2_open(struct inode *minode, struct file *mfile) 
{	
	if(usage_count != 0) return -EBUSY;

	usage_count = 1;

	return 0;
}

int hw2_release(struct inode *minode, struct file *mfile) 
{
	usage_count = 0;

	return 0;
}

int __init hw2_init(void)
{
	int result;

	result = register_chrdev(MAJOR_NUMBER, HW2_DEVICE_DRIVER, &hw2_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}

	led_addr = ioremap(LED_ADDRESS, 0x1);
	fnd_addr = ioremap(FND_ADDRESS, 0x4);
	dot_addr = ioremap(DOT_ADDRESS, 0x10);
	lcd_addr = ioremap(LCD_ADDRESS, 0x32);
	init_timer(&(my_timer.timer));
	printk("init module, %s major number : %d\n", HW2_DEVICE_DRIVER, MAJOR_NUMBER);

	return 0;
}

void __exit hw2_exit(void) 
{
	iounmap(led_addr);
	iounmap(fnd_addr);
	iounmap(dot_addr);
	iounmap(lcd_addr);
	del_timer_sync(&my_timer.timer);
	unregister_chrdev(MAJOR_NUMBER, HW2_DEVICE_DRIVER);
	printk("rmmod module\n");
}

module_init(hw2_init);
module_exit(hw2_exit);

MODULE_LICENSE("GPL");
