#include "CartesianRobot.h"

////////////////////////////////////////////////////////////////////////////////
//                          Constructor & Destructor                          //
////////////////////////////////////////////////////////////////////////////////
CartesianRobot::CartesianRobot(
        PinName x_step, PinName x_dir, PinName x_en, 
        PinName y_step, PinName y_dir, PinName y_en,
        PinName z_step, PinName z_dir, PinName z_en
) : _x_stepper(x_step, x_dir, x_en),
    _y_stepper(y_step, y_dir, y_en),
    _z_stepper(z_step, z_dir, z_en)
{
    _buffer_size = 0;
}

CartesianRobot::~CartesianRobot(){}


////////////////////////////////////////////////////////////////////////////////
//                             Setters & Getters                              //
////////////////////////////////////////////////////////////////////////////////
void CartesianRobot::setXSpeed(float speed){_x_stepper.setSpeed(speed);}
void CartesianRobot::setYSpeed(float speed){_y_stepper.setSpeed(speed);}
void CartesianRobot::setZSpeed(float speed){_z_stepper.setSpeed(speed);}

void CartesianRobot::setXAcc(float acc){_x_stepper.setAcceleration(acc);}
void CartesianRobot::setYAcc(float acc){_y_stepper.setAcceleration(acc);}
void CartesianRobot::setZAcc(float acc){_z_stepper.setAcceleration(acc);}

void CartesianRobot::setXDec(float dec){_x_stepper.setDeceleration(dec);}
void CartesianRobot::setYDec(float dec){_y_stepper.setDeceleration(dec);}
void CartesianRobot::setZDec(float dec){_z_stepper.setDeceleration(dec);}

void CartesianRobot::enable(void)
{
    _x_stepper.enable();
    _y_stepper.enable();
    _z_stepper.enable();
}

void CartesianRobot::disable(void)
{
    _x_stepper.disable();
    _y_stepper.disable();
    _z_stepper.disable();
}

bool CartesianRobot::enabled(void)
{
    return _x_stepper.enabled() && _y_stepper.enabled() && _z_stepper.enabled();
}

bool CartesianRobot::stopped(void)
{
    return _x_stepper.stopped() && _y_stepper.stopped() && _z_stepper.stopped();
}

int CartesianRobot::getXPosition(void){return _x_stepper.getPosition();}
int CartesianRobot::getYPosition(void){return _y_stepper.getPosition();}
int CartesianRobot::getZPosition(void){return _z_stepper.getPosition();}

int CartesianRobot::getXLimit(void){return _x_stepper.getPositionLimit();}
int CartesianRobot::getYLimit(void){return _y_stepper.getPositionLimit();}
int CartesianRobot::getZLimit(void){return _z_stepper.getPositionLimit();}


////////////////////////////////////////////////////////////////////////////////
//                                Calibration                                 //
////////////////////////////////////////////////////////////////////////////////
void CartesianRobot::setOrigin(void)
{
    _x_stepper.setPositionZero();
    _y_stepper.setPositionZero();
    _z_stepper.setPositionZero();
}

void CartesianRobot::setLimits(void)
{
    _x_stepper.setPositionLimit();
    _y_stepper.setPositionLimit();
    _z_stepper.setPositionLimit();
}


////////////////////////////////////////////////////////////////////////////////
//                                 Movement                                   //
////////////////////////////////////////////////////////////////////////////////
void CartesianRobot::moveToXYZ(
    int x_pos, int y_pos, int z_pos, 
    float x_speed, float y_speed, float z_speed
){
    if (_buffer_size < BUFFER_MAX_SIZE){        
        _buffer_mutex.lock();
        _x_buffer_pos[_buffer_size] = x_pos;
        _y_buffer_pos[_buffer_size] = y_pos;
        _z_buffer_pos[_buffer_size] = z_pos;
        _x_buffer_speed[_buffer_size] = x_speed;
        _y_buffer_speed[_buffer_size] = y_speed;
        _z_buffer_speed[_buffer_size] = z_speed;
        _buffer_size++;
        _buffer_mutex.unlock();
    }
}

void CartesianRobot::goToXYZ(int x_pos, int y_pos, int z_pos){
    _x_stepper.goTo(x_pos);
    _y_stepper.goTo(y_pos);
    _z_stepper.goTo(z_pos);
}

void CartesianRobot::moveX(int steps){_x_stepper.move(steps);}
void CartesianRobot::moveY(int steps){_y_stepper.move(steps);}
void CartesianRobot::moveZ(int steps){_z_stepper.move(steps);}


////////////////////////////////////////////////////////////////////////////////
//                             Buffer Management                              //
////////////////////////////////////////////////////////////////////////////////
void CartesianRobot::startManager(void) {
    _thread = new Thread(manager, this);
}

void CartesianRobot::manager(const void *p){
    CartesianRobot* self = (CartesianRobot*)p;
    int x_pos, y_pos, z_pos;
    float x_speed, y_speed, z_speed;
    while(1){
        if (self->_buffer_size){
            x_pos = self->_x_buffer_pos[0];
            y_pos = self->_y_buffer_pos[0];
            x_speed = self->_x_buffer_speed[0];
            y_speed = self->_y_buffer_speed[0];
            z_pos = self->_z_buffer_pos[0];
            z_speed = self->_z_buffer_speed[0];
            while(!self->stopped());
            self->setXSpeed(x_speed);
            self->setYSpeed(y_speed);
            self->setZSpeed(z_speed);
            self->goToXYZ(x_pos, y_pos, z_pos);

            self->_buffer_mutex.lock();
            for (int i=0; i<self->_buffer_size && i<BUFFER_MAX_SIZE-1; i++){
                self->_x_buffer_pos[i] = self->_x_buffer_pos[i+1];
                self->_y_buffer_pos[i] = self->_y_buffer_pos[i+1];
                self->_x_buffer_speed[i] = self->_x_buffer_speed[i+1];
                self->_y_buffer_speed[i] = self->_y_buffer_speed[i+1];
                self->_z_buffer_pos[i] = self->_z_buffer_pos[i+1];
                self->_z_buffer_speed[i] = self->_z_buffer_speed[i+1];
            }
            self->_buffer_size--;
            self->_buffer_mutex.unlock();
        }
    }
}
