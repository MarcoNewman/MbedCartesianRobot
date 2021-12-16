#ifndef CARTESIANROBOT_H
#define CARTESIANROBOT_H

#include "Stepper.h"
#include "rtos.h"

#define BUFFER_MAX_SIZE 10

class CartesianRobot
{
public:
    // Constructor
    CartesianRobot(
        PinName x_step, PinName x_dir, PinName x_en, 
        PinName y_step, PinName y_dir, PinName y_en,
        PinName z_step, PinName z_dir, PinName z_en
    );
    // Destructor
    ~CartesianRobot();
    
    // Add motion to buffer
    void moveToXYZ(
        int x_pos, int y_pos, int z_pos, 
        float x_speed, float y_speed, float z_speed
    );
    // Move robot to absolute position
    void goToXYZ(int x_pos, int y_pos, int z_pos);
    // Move motors a set number of steps
    void moveX(int steps);
    void moveY(int steps);
    void moveZ(int steps);
    
    // Set motor speeds [steps/sec]
    void setXSpeed(float speed);
    void setYSpeed(float speed);
    void setZSpeed(float speed);
    // Set motor acceleration ramps [steps/sec²] (0 = infinite acceleration)
    void setXAcceleration(float acc);
    void setYAcceleration(float acc);
    void setZAcceleration(float acc);
    // Set motor deceleration ramps [steps/sec²] (0 = infinite deceleration)
    void setXDeceleration(float dec);
    void setYDeceleration(float dec);
    void setZDeceleration(float dec);
    
    // Enable motor drivers
    void enable(void);
    // Disable motor drivers
    void disable(void);
    // Check if motor drivers are enabled
    bool enabled(void);
    // Check if motors are stopped
    bool stopped(void);
    // Get absolute positions from origin
    int getXPosition(void);
    int getYPosition(void);
    int getZPosition(void);
    
    // Set origin for absolute motion
    void setOrigin(void);
    // Set limits for absolute motion
    void setLimits(void);
    // Get limits for absolute motion
    int getXLimit(void);
    int getYLimit(void);
    int getZLimit(void);
    
    // Start buffer management thread
    void startManager(void);
    
 private:
    Stepper _x_stepper;                         // x axis stepper object
    Stepper _y_stepper;                         // y axis stepper object
    Stepper _z_stepper;                         // z axis stepper object
    
    Mutex _buffer_mutex;                        // thread safe buffer
    int _buffer_size;                           // current buffer size
    int _x_buffer_pos[BUFFER_MAX_SIZE];         // x position buffer
    int _y_buffer_pos[BUFFER_MAX_SIZE];         // y position buffer
    int _z_buffer_pos[BUFFER_MAX_SIZE];         // z position buffer
    float _x_buffer_speed[BUFFER_MAX_SIZE];     // x speed buffer
    float _y_buffer_speed[BUFFER_MAX_SIZE];     // y speed buffer
    float _z_buffer_speed[BUFFER_MAX_SIZE];     // z speed buffer

    // buffer management
    Thread* _thread;
    static void manager(void const *p);
    
};

#endif