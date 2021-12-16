#include "Stepper.h"

////////////////////////////////////////////////////////////////////////////////
//                                 Constructor                                //
////////////////////////////////////////////////////////////////////////////////
Stepper::Stepper(PinName step, PinName dir, PinName en):
    _step(step) , _dir(dir), _en(en)
{
    _en = 0;
    _step = 1;
    _state = STOP;
    _pos = 0;
    _lim = 0;
    _steps = 0;
    _spd = 400;
    _dt0 = 0;
}


////////////////////////////////////////////////////////////////////////////////
//                             Setters & Getters                              //
////////////////////////////////////////////////////////////////////////////////
void Stepper::setPositionZero(void){_pos = 0;}

void Stepper::setPositionLimit(void){_lim = _pos;}

int Stepper::getPositionLimit(void){return _lim;}

void Stepper::enable(void){_en = 0;}

void Stepper::disable(void){_en = 1;}

bool Stepper::enabled(void){return !_en;}

bool Stepper::stopped(void){return (_state == STOP) ? true : false;}

int Stepper::getPosition(void){return _pos;}

void Stepper::setSpeed(float speed)
{
    _spd = (speed<0) ? -speed : speed;  // speed must be unsigned
    if(_spd)_dtmin = 1000000/_spd;  // find min delay (max spd)
}

float Stepper::getSpeed(void){return _spd;}

void Stepper::setAcceleration(float acc)
{
    _acc = (acc<0) ? -acc : acc;  // acceleration must be unsigned
    if(_acc)
        _dt0 = 676000 * sqrt(2.0/_acc);
}
 
float Stepper::getAcceleration(void){return _acc;}

void Stepper::setDeceleration(float dec)
{
    _dec = (dec<0) ? -dec : dec;  // deceleration must be unsigned
}
 
float Stepper::getDeceleration(void){return _dec;}


////////////////////////////////////////////////////////////////////////////////
//                                 Movement                                   //
////////////////////////////////////////////////////////////////////////////////
void Stepper::stop(void)
{   
    _step = 1;
    remove();           //stop timer
    _state = STOP;      //update state machine 
    _steps = 0;         //reset total steps per move
}

void Stepper::move(int steps)
{
    if(!steps || !_spd) return;
    if(steps<0)  // find direction
    {
        _dir = CCW;  // set output pin direction value
        _steps = -steps;  // total steps per move must be unsigned
    }
    else
    {
        _dir = CW;  // set output pin direction value
        _steps = steps;  // total steps per move
    }
    handler();  // start thread
}
 
void Stepper::goTo(int position)
{
    if (_lim == 0 || position <= _lim) 
        move(position-_pos);  // absolute to relative transformation
    else
        move(_lim-_pos);  // move to max position
}


////////////////////////////////////////////////////////////////////////////////
//                             Protected Methods                              //
////////////////////////////////////////////////////////////////////////////////
void Stepper::handler(void)
{
    static float i;
    
    switch(_state)
    {
        case STOP:
            _n = 0;  // reset setp counter (motor stopped)
            
            // if first step faster than max speed step
            if(_dt0 <= _dtmin || !_acc)  
            {
                _dtn = _dtmin;  // delay = delaymin
                _state = CRUISE;  // start cruise
            }
            else
            {
                _dtn = _dt0;  // set first delay
                _state = ACCEL;  // start acceleration
            }
 
            if(_steps)   // if finite move required
            {
                // number of steps to reach max speed 
                unsigned int nToSpeed = nTo(_spd,_acc); 
                // number of steps until deceleration   
                _nStartDec = (_steps * _dec) / (_dec + _acc);  
                if(_nStartDec > nToSpeed)  // if speed can be reached
                    _nStartDec = _steps - ((nToSpeed*_acc)/_dec);       
            }
            i = _dtn;
        break;
        
        case ACCEL:
            i -= i*2.0 / ((_n<<2)+1);  // find next delay
            _dtn = i;
            
            if((unsigned int)_dtn <= _dtmin)  // if max speed reached
            {
                 _dtn = _dtmin;
                 i = _dtn;
                _state = CRUISE;  // start cruise
            }
            if(_steps && _dec && _n >= _nStartDec)
                _state = DECEL;  // start deceleration
        break;
        
        case CRUISE:
            if(_steps && _dec && _n >= _nStartDec)
                _state = DECEL;  // start deceleration
        break;
        
        case DECEL:
            i += (i*2.0) / (((_steps-_n)<<2)+1);  // find next delay
            _dtn = i;
        break;    
    }
    
    _step=0;
    
    if(!_n) 
        insert(_dtn + us_ticker_read());  // start timer @ first delay
    else 
        insert(event.timestamp+(unsigned int)_dtn);
    
    _n++;  // increment step counter
    _pos += (_dir<<1)-1;  // set new position +1 if cw; -1 if ccw
    _step = 1;  // toggle step out pin
 
    if(_steps && _n >= _steps)  // check for motor stop
        stop();  
}

unsigned int Stepper::nTo(float speed,float acc)
{
    if(speed<0)speed = -speed;
    if(acc<0)acc = -acc;
    
    // step number n as a function of speed & acceleration
    return (!acc || !speed) ? 0 : (speed * speed) / (2 * acc);  
}