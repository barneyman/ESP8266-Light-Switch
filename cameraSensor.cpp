#include "cameraSensor.h"


#ifdef ARDUINO_ARCH_ESP32

#ifdef WROVER_CAM

//WROVER-KIT PIN Map
#define CAM_PIN_PWDN    -1 //power down is not used
#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_XCLK    21
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27

#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      19
#define CAM_PIN_D2      18
#define CAM_PIN_D1       5
#define CAM_PIN_D0       4
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22


#else

#define CAM_PIN_PWDN     32
#define CAM_PIN_RESET    -1
#define CAM_PIN_XCLK      0
#define CAM_PIN_SIOD     26
#define CAM_PIN_SIOC     27

#define CAM_PIN_D7       35
#define CAM_PIN_D6       34
#define CAM_PIN_D5       39
#define CAM_PIN_D4       36
#define CAM_PIN_D3       21
#define CAM_PIN_D2       19
#define CAM_PIN_D1       18
#define CAM_PIN_D0        5

#define CAM_PIN_VSYNC    25
#define CAM_PIN_HREF     23
#define CAM_PIN_PCLK     22

#define LED_PIN           33 // Status led
#define LED_ON           LOW // - Pin is inverted.
#define LED_OFF         HIGH //
#define LAMP_PIN           4 // LED FloodLamp.


#endif


static camera_config_t camera_config = {
    .pin_pwdn  = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sscb_sda = CAM_PIN_SIOD,
    .pin_sscb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,//YUV422,GRAYSCALE,RGB565,JPEG
    //.frame_size = FRAMESIZE_UXGA,//QQVGA-QXGA Do not use sizes above QVGA when not JPEG
    .frame_size = FRAMESIZE_VGA,//QQVGA-QXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 25, //0-63 lower number means higher quality
    .fb_count = 1 //if more than one, i2s runs in continuous mode. Use only with JPEG
};

esp_err_t  esp32Cam::initialiseCam()
{
    if(CAM_PIN_PWDN != -1){
            pinMode(CAM_PIN_PWDN, OUTPUT);
            digitalWrite(CAM_PIN_PWDN, LOW);
        }

        //initialize the camera
        esp_err_t err = esp_camera_init(&camera_config);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Camera Init Failed");
            return err;
        }

        return ESP_OK;
}


int esp32Cam::requestFrame()
{
    if(m_frameBuffer)
    {
        if(m_dblog) m_dblog->println(debug::dbInfo,"previous framebuffer discarded");
        // let go of this first
        releaseFrameBuffer();
    }

    esp_err_t res = ESP_OK;

    m_frameBuffer = esp_camera_fb_get();

    if(!m_frameBuffer)
    {
        if(m_dblog) m_dblog->println(debug::dbError,"NULL framebuffer returned");
        return 0;
    }

    if(m_dblog) m_dblog->printf(debug::dbInfo,"framebuffer size=%u (%u x %u) @ 0x%x\n\r", m_frameBuffer->len,m_frameBuffer->width,m_frameBuffer->height,m_frameBuffer->buf);


    return m_frameBuffer->len;


}

// don't forget to free your buffer!!
bool esp32Cam::fetchFrame(uint8_t **toHere, size_t *len)
{
    if(!m_frameBuffer)
    {
        if(m_dblog) m_dblog->println(debug::dbError,"framebuffer not available");
        return false;
    }

    if(!toHere || !len)
    {
        if(m_dblog) m_dblog->println(debug::dbError,"don't pass me NULLs!");
        return false;

    }
    *len=0;

    *toHere=(uint8_t *)malloc(m_frameBuffer->len);
    *len=m_frameBuffer->len;

    if(!(*toHere))
    {
        if(m_dblog) m_dblog->printf(debug::dbError,"image malloc of %ld failed!\r",m_frameBuffer->len);
        releaseFrameBuffer();
        return false;
    }

    memcpy(*toHere, m_frameBuffer->buf, m_frameBuffer->len);

/*
    if(!frame2jpg(m_frameBuffer,20,toHere,len))
    {
        if(m_dblog) m_dblog->println(debug::dbError,"error in frame2jpg");
        return false;
    }
*/

    releaseFrameBuffer();

    return true;

}



uint8_t smallestJPEG[]={ 0xFF,0xD8,0xFF,0xE0,0x00,0x10,0x4A,0x46,0x49,0x46,0x00,0x01,0x01,0x01,0x00,0x48,0x00,0x48,0x00,0x00,
0xFF,0xDB,0x00,0x43,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC2,0x00,0x0B,0x08,0x00,0x01,0x00,0x01,0x01,0x01,
0x11,0x00,0xFF,0xC4,0x00,0x14,0x10,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xFF,0xDA,0x00,0x08,0x01,0x01,0x00,0x01,0x3F,0x10 };

int fakeCamera::requestFrame()
{
    return sizeof(smallestJPEG);
}

bool fakeCamera::fetchFrame(uint8_t **toHere, size_t *len)
{
    *toHere=(uint8_t*)malloc(sizeof(smallestJPEG));

    if(!(*toHere))
    {
        return false;
    }

    memcpy(*toHere,smallestJPEG,*len=sizeof(smallestJPEG));
    

    return true;
}




#endif