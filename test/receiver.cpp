

// Wifi settings
const char* ssid = "JCBS-Schüler";
const char* password = "S1,16DismdEn;deieKG!";


// set this in AudioConfig.h or here after installing https://github.com/pschatzmann/arduino-libhelix.git
#define USE_HELIX 

#include "AudioTools.h"
#include "AudioLibs/AudioKit.h"


AudioKitStream kit;
URLStream url(ssid,password);
EncodedAudioStream dec(&kit, new AACDecoderHelix()); // Decoding stream
StreamCopy copier(dec, url); // copy url to decoder


void setup(){
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);  

  // setup i2s
 auto cfg = kit.defaultConfig(TX_MODE);
  cfg.sd_active = false;
  kit.begin(cfg);
  // setup I2S based on sampling rate provided by decoder
  dec.setNotifyAudioChange(kit);
  dec.begin();

// mp3 radio
  url.begin("http://192.168.2.78/","audio/aac");

}

void loop(){
  copier.copy();
}
