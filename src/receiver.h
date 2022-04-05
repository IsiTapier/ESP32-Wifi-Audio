
#ifndef RECEIVER
#define RECEIVER
// Wifi settings
// const char* ssid = "JCBS-Schüler";
// const char* password = "S1,16DismdEn;deieKG!";
const char* ssid = "NetFrame";
const char* password = "87934hzft9oeu4389nv8o437893hf978";
// const char* ssid = "PSM_Gast";
// const char* password = "2016Daheim12";
// const char* ssid = "ESP32";
// const char* password = "12345678";


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
 auto config = kit.defaultConfig(TX_MODE);
  config.sd_active = false;
  config.sample_rate = 22050; 
  config.bits_per_sample = 16;
  config.default_actions_active = false; 
  config.sd_active = false;
  config.channels = 2; 
  kit.begin(config);
  // setup I2S based on sampling rate provided by decoder
  dec.setNotifyAudioChange(kit);
  dec.begin();

// mp3 radio
  url.begin("http://192.168.178.71/","audio/aac");

}

void loop(){
  copier.copy();
}

#endif