#include "Joystick.hpp"

/****************************************************************************/
Joystick::Joystick(Accelerometer& accelerometer, float maxTilt)
: accel(accelerometer)
{
    _maxTilt = constrain(maxTilt, 5.f, 45.f);
    if(_maxTilt != maxTilt) {
        Serial.println("Max Tilt Constrained to [5°, 45°]");
        msgPause(Serial);
    }
    _zero = Vec2f();
    _init = true;
}
/****************************************************************************/
bool Joystick::begin(){
    if(!_init){ return ErrorMsg("Joystick not initialised!"); }
    if(!accel.begin()){ return ErrorMsg("Accel-Joystick integration problem"); }
    return true;
}
/****************************************************************************/
bool Joystick::calibrate(Vec2f manual){
    // Manual calibration - use provided offsets
    if (!manual.hasNaN()) {
        _zero = manual; 
        Serial.printf(
            "📐 Manual Calibration: Pitch = %.2f°, Roll = %.2f°\n",
            manual[0], manual[1]
        );
        //msgPause();
        return true; 
    } 
    // Automatic calibration to set offsets
    else {
        Serial.println("🎯 Calibrating...do not move!");
        Serial.flush(); delay(100);
        const uint16_t samples = 128;
        uint16_t sampleCount = 0;
        const uint32_t dt = getInterval();
        Vec2f vec; 
        for (uint16_t k = 0; k < samples; k++) {
            _zero += readRaw() ? _tilt : Vec2f();
            sampleCount += 1; 
            delay(dt);
        }
        if(sampleCount >= 1){ 
            _zero /= static_cast<float>(sampleCount);
            Serial.printf(
                "✅ Calibration Done: Pitch = %.2f°, Roll = %.2f°, Samples = %d\n",
                _zero[0], _zero[1], sampleCount
            );
            return true; 
        } else {
            return ErrorMsg("Calibration failed! No valid samples.");
        }
    }
}
/****************************************************************************/
bool Joystick::readRaw() {
    String msg = "Joystick not reading raw!";
    if(!accel.read()){ return ErrorMsg(msg); }
    _tilt = tiltProjection(accel.coords);
    return true; 
}
/****************************************************************************/
bool Joystick::readCalibrated(){
    String msg = "Joystick not reading calibrated!";
    if(!readRaw()) { return ErrorMsg(msg); }
    _tilt -= _zero;
    return true; 
}
/****************************************************************************/
bool Joystick::readClipped(){
    String msg = "Joystick not reading filtered!";
    if(!readCalibrated()) { return ErrorMsg(msg); }
    // 1. We clip first! 
    _tilt[0] = constrain(_tilt[0], -_maxTilt, _maxTilt);
    _tilt[1] = constrain(_tilt[1], -_maxTilt, _maxTilt);
    // 2. "Project" angles to XY via linar response
    _tilt /= _maxTilt;
    return true;
}
/****************************************************************************/
Vec2f Joystick::read(){
    if(!readClipped()) { return Vec2f(); }
    return _tilt; 
}
/****************************************************************************/
void Joystick::print(Stream& stream){
    readClipped();
    Vec3f out = {accel._tic, _tilt[0], _tilt[1]};
    out.print(Serial);
    
    // WORKING!
 
    //float toc = 1e-3 * static_cast<float>(accel._tic);
    //stream.printf("%d\t%.2f\t%.2f\n", accel._tic, _tilt[0], _tilt[1]);
}
/****************************************************************************/