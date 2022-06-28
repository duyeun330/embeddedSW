실행 방법

1. sudo -s을 통해 터미널 권한

2. cd /work/achroimx_kernel 에서 다음을 실행
 - source /root/.bashrc
 - export ARCH=arm
 - make achroimx_defconfig

3. 새로운 터미널에서 sudo minicom -s 실행 및 보드 켜기

4. 이전 터미널에서 20161664 폴더내로 이동

5. app과 module 폴더에서 make 실행후 실행파일과 디바이스 드라이버 파일을 전송
 - app 폴더: make -> adb push app /data/local/tmp
 - module 폴더: make -> adb push interrupt.ko /data/local/tmp

6. minicom에서 cd /data/local/tmp로 이동

7. kernel이 잘 작동하는지 확인하기 위해 다음 명령어 실행
 - echo "7 6 1 7" > /proc/sys/kernel/printk

8. insmod interrupt.ko 실행

9. mknod /dev/stopwatch c 242 0 실행

10. ./app 수행
