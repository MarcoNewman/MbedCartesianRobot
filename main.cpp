#include "mbed.h"
#include "rtos.h"
#include "uLCD_4DGL.h"
#include "CartesianRobot.h"

Serial pc(USBTX,USBRX);

DigitalIn enable(p13);
DigitalIn set(p14);
DigitalIn controlX(p15);
DigitalIn controlY(p16);
DigitalIn controlZ(p17);

CartesianRobot robot(p22,p23,p24, p25,p26,p27, p28,p29,p30);

uLCD_4DGL uLCD(p9,p10,p11);
Mutex lcd_mutex;

// manage motor driver enable
void enable_switch(void const *args){
    while(1){
        if (!enable && robot.enabled())
            robot.disable();
        else if (enable && !robot.enabled())
            robot.enable();
    }
}

// robot position monitoring
void position_monitor(void const *args){
    int x_pos, y_pos, z_pos;
    while(1){
        x_pos = robot.getXPosition()/8;
        y_pos = robot.getYPosition()/8;
        z_pos = robot.getZPosition()/8;
        lcd_mutex.lock();
        uLCD.locate(0,5);
        uLCD.printf("                 ");
        uLCD.locate(0,5);
        uLCD.printf("(%d, %d, %d)", x_pos, y_pos, z_pos);  
        lcd_mutex.unlock();
        Thread::wait(200);
    }
} 

int main()
{
    // configure PB and switch internal PullDown
    enable.mode(PullDown);
    set.mode(PullDown);
    controlX.mode(PullDown);
    controlY.mode(PullDown);
    controlZ.mode(PullDown);
    wait(1);
    
    // set robot speeds, accelerations, and decelerations
    robot.setXSpeed(1200);
    robot.setYSpeed(1200);
    robot.setZSpeed(2400);
    robot.setXAcceleration(0);
    robot.setYAcceleration(0);
    robot.setZAcceleration(0);
    robot.setXDeceleration(0);
    robot.setYDeceleration(0);
    robot.setZDeceleration(0);
    
    // baud rate to max for fast display
    uLCD.baudrate(3000000);
    uLCD.display_control(PORTRAIT);
    
    // start robot, enable, and position monitoring threads
    robot.startManager();
    Thread t1(thread1);
    Thread t2(thread2);
    
    // set the origin position
    lcd_mutex.lock();
    uLCD.locate(0,0);
    uLCD.printf("Press set when at desired 0 pos...");
    while(!set);
    robot.setOrigin();
    uLCD.cls();
    
    // set upper limits
    uLCD.printf("Press set when at desired max pos...");
    lcd_mutex.unlock();
    while(!set){
        if (robot.stopped() && controlX)
            robot.moveX(80);
        if (robot.stopped() && controlY)
            robot.moveY(80);
        if (robot.stopped() && controlZ)
            robot.moveZ(80);    
    }
    robot.setLimits();
    
    // prepare for motion control
    lcd_mutex.lock();
    uLCD.cls();
    uLCD.printf("Enter Positions...\nFormat:\nx,y,z");
    uLCD.locate(0,11);
    uLCD.printf("Limits:\n(%d, %d, %d)", 
        robot.getXLimit()/8, robot.getYLimit()/8, robot.getZLimit()/8
    );
    lcd_mutex.unlock();
    
    // buffer movements as default speed
    int x, y, z;
    int x_s = 600;
    int y_s = 600;
    int z_s = 1000;
    while(1){
        if (pc.readable()){
            lcd_mutex.lock();
            pc.scanf("%d,%d,%d", &x, &y, &z);
            pc.printf("Position Buffered\n");
            pc.printf("x=%d, y=%d, z=%d\n", x, y, z);
            lcd_mutex.unlock();
            robot.moveToXYZ(x*8,y*8,z*8,(float)x_s,(float)y_s,(float)z_s);
        }
    }
}