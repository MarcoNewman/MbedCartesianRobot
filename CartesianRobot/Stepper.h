#ifndef STEPPER_H
#define STEPPER_H

#include "mbed.h"

class Stepper : public TimerEvent
{
public:
    // Create Stepper instance connected to pins step, direction, and enable
    Stepper(PinName step, PinName dir, PinName en);
    
    // Set origin for motor absolute movement
    void setPositionZero(void);
    // Set limit for motor absolute movement
    void setPositionLimit(void);
    // Get limit of motor absolute movement
    int getPositionLimit(void);
    
    // Stop motor
    void stop(void);
    // Move motor to absolute position
    void goTo(int position);
    // Move motor a set number of steps
    void move(int steps);
    
    // Set rotation speed [steps/sec]
    void setSpeed(float speed);
    // Set acceleration ramp [steps/sec²] (0 = infinite acceleration)
    void setAcceleration(float acc);
    // Set deceleration ramp [steps/sec²] (0 = infinite deceleration)
    void setDeceleration(float dec);

    // Enable motor driver
    void enable(void);
    // Disable motor driver
    void disable(void);
    // Check if motor driver is enabled
    bool enabled(void);
    // Check if motor is stopped
    bool stopped(void);
    // Get absolute position from origin
    int getPosition(void);
    // Get speed [steps/sec]
    float getSpeed(void);
    // Get acceperation [steps/sec²]
    float getAcceleration(void);
    // Get deceleration [steps/sec²]
    float getDeceleration(void);
    
    // Enum for direction
    typedef enum {CW=1,CCW=0} direction;
    
protected:
    unsigned int nTo(float speed,float acc);
    
 private:
    DigitalOut _step;                       // output step pin 
    DigitalOut _dir;                        // output direction pin
    DigitalOut _en;                         // output enable pin 
    
    virtual void handler();
    enum {STOP,ACCEL,CRUISE,DECEL}_state;   // motor state
    
    int _pos;                               // motor position
    int _lim;                               // motor limit
    float _spd;                             // speed [step/s]
    float _acc;                             // acceleration [step/s²]
    float _dec;                             // decceleration [step/s²]
    
    unsigned int _steps;                    // nbr total of steps per mov
    
    unsigned int _dt0;                      // initial delay [µs]
    unsigned int _dtmin;                    // delay minimum [µs]
    unsigned int _dtn;                      // current delay
    
    unsigned int _n;                        // steps counters
    unsigned int _nStartDec;                // steps to decelerate
    
};
 
#endif