#include <Arduino.h>
#include <debugLogger.h>
#include <ArduinoJson.h>

#include "announce.h"

#include <queue>
#include <functional>

class baseCamera : public baseThing
{

public:

    baseCamera(debugBaseClass*dbg):baseThing(dbg)
    {

    }

    virtual bool InitialisedOk()=0;
    virtual int requestFrame()=0;
    virtual bool fetchFrame(uint8_t **toHere, size_t *len)=0;



};

#ifdef ARDUINO_ARCH_ESP32

//#include <freertos/include/freertos/semphr.h>

class espCamera : public baseCamera
{
public:

    espCamera(debugBaseClass*dbg):baseCamera(dbg)
    {
        m_qSemaphore=xSemaphoreCreateMutex();
    }

    virtual void DoWork()
    {
        if(m_requests.size())
        {
            // call the front lambda
            while(xSemaphoreTake(m_qSemaphore,10)!=true)
                yield();
            std::function<void(baseCamera*)> fn=m_requests.front();
            m_requests.pop();
            xSemaphoreGive(m_qSemaphore);

            fn(this);
        }
    }

    void AddWork(std::function<void(baseCamera*)> fn)
    {
        while(xSemaphoreTake(m_qSemaphore,0)!=true)
            yield();
        m_requests.push(fn);
        xSemaphoreGive(m_qSemaphore);
    }


protected:

    std::queue<std::function<void(baseCamera*)>> m_requests;
    SemaphoreHandle_t m_qSemaphore;

};



class fakeCamera : public espCamera
{
public:

    fakeCamera(debugBaseClass*dbg):espCamera(dbg)
    {
        thingName="fakeCamera";
    }

    virtual bool InitialisedOk()
    {
        return true;
    }

    virtual int requestFrame();

    virtual bool fetchFrame(uint8_t **toHere, size_t *len);

};




#include "esp_camera.h"

class esp32Cam:public espCamera
{

public:

    esp32Cam(debugBaseClass*dbg):espCamera(dbg),m_frameBuffer(NULL)
    {
        thingName="esp32Cam";
        m_initErr=initialiseCam();
    }

    virtual bool InitialisedOk()
    {
        return m_initErr==ESP_OK;
    }

    virtual int requestFrame();
    virtual bool fetchFrame(uint8_t **toHere, size_t *len);


protected:

    esp_err_t  initialiseCam();

    esp_err_t m_initErr;

private:

    camera_fb_t *m_frameBuffer;

    void releaseFrameBuffer()
    {
        if(m_frameBuffer)
        {
            esp_camera_fb_return(m_frameBuffer);
            m_frameBuffer=NULL;
        }

    }

};

#endif