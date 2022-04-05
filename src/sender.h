#pragma once

// Wifi settings
// const char* ssid = "JCBS-Schüler";
// const char* password = "S1,16DismdEn;deieKG!";
const char* ssid = "NetFrame";
const char* password = "87934hzft9oeu4389nv8o437893hf978";
// const char* ssid = "PSM_Gast";
// const char* password = "2016Daheim12";
// const char* ssid = "ESP32";
// const char* password = "12345678";

/**
 * @file streams-audiokit-webserver_aac.ino
 *
 *  This sketch reads sound data from the AudioKit. The result is provided as AAC stream which can be listened to in a Web Browser
 *
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
#define USE_FDK

#include "AudioTools.h"
#include "AudioLibs/AudioKit.h"

AudioKitStream kit;    
AACEncoderFDK *fdk=nullptr;
AudioEncoderServer *server=nullptr;  

// Arduino setup
void setup(){
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);  
  // Defining Loglevels for the different libraries
  // LOGLEVEL_FDK = FDKInfo; 
  LOGLEVEL_AUDIOKIT = AudioKitInfo;
  
  // setup and configure fdk
  fdk = new AACEncoderFDK();  
  fdk->setAudioObjectType(2);  // AAC low complexity
  fdk->setOutputBufferSize(1024); // decrease output buffer size
  fdk->setVariableBitrateMode(0); // low variable bitrate
  server = new AudioEncoderServer(fdk,ssid,password);  


  // start i2s input with default configuration
  Serial.println("starting AudioKit...");
  auto config = kit.defaultConfig(RX_MODE);
  config.input_device = AUDIO_HAL_ADC_INPUT_LINE2;
  config.sample_rate = 22050; 
  config.bits_per_sample = 16;
  config.default_actions_active = false; 
  config.channels = 2; 
  config.sd_active = false;
  kit.begin(config);
  Serial.println("AudioKit started");

  // start data sink
  server->begin(kit, config);
  Serial.println("Server started");

}

// Arduino loop  
void loop() {
  // Handle new connections
  server->doLoop();  
}
