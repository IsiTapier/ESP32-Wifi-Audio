
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
// const char* ssid = "GodiTechnik";
// const char* password = "SoundcraftVi400!";

// set this in AudioConfig.h or here after installing https://github.com/pschatzmann/arduino-libhelix.git
#define USE_HELIX 

#include "AudioTools.h"
#include "AudioLibs/AudioKit.h"
#include "Arduino.h"

AudioKitStream kit;
URLStream url(ssid,password, 16*1024);
EncodedAudioStream dec(&kit, new AACDecoderHelix()); // Decoding stream
StreamCopy copier(dec, url, 16*1024); // copy url to decoder
// ICYStream urlStream(ssid, password, 64*1024);
// AudioSourceURL source(urlStream, urls, "audio/aac");
// AACDecoderHelix decoder;
// AudioPlayer player(source, kit, decoder);

// bool playState = true;

// void next(bool, int, void*) {
//     player.next();
//   Serial.println("next");
// }

// void previous(bool, int, void*) {
//     player.previous();
//   Serial.println("pevious");
// }

void setup(){
  Serial.begin(115200);
  // AudioLogger::instance().begin(Serial, AudioLogger::Info);
  Serial.println("Serial started");

  Serial.println("starting AudioKit...");
  auto config = kit.defaultConfig(TX_MODE);
  // config.input_device = AUDIO_HAL_ADC_INPUT_LINE2;
  config.output_device = AUDIO_HAL_DAC_OUTPUT_ALL;
  config.sample_rate = 8000; 
  config.bits_per_sample = 16;
  config.channels = 2;
  config.default_actions_active = true;
  config.sd_active = false;
  kit.begin(config);
  Serial.println("AudioKit started");

  // player.setVolume(1.);
  // player.begin();

  // kit.addAction(PIN_KEY4, next);
  // kit.addAction(PIN_KEY3, previous);

  // auto down = [](bool,int,void*) { AudioKitStream::actionVolumeDown(true, -1, nullptr); Serial.println("Volume down"); };
  // kit.addAction(PIN_KEY5, down);
  // auto up = [](bool,int,void*) { AudioKitStream::actionVolumeUp(true, -1, nullptr ); Serial.println("Volume up"); };
  // kit.addAction(PIN_KEY6, up);
  // auto play = [](bool,int,void*) { AudioKitStream::actionStartStop(true, -1, nullptr ); playState=!playState; Serial.println(playState?"Continue playing" : "Stop playing"); };
  // kit.addAction(PIN_KEY1, play);

  dec.setNotifyAudioChange(url);
  dec.begin();

  // mp3 radio
  url.begin("http://192.168.178.98/","audio/aac");
}

unsigned long lastAvailable = millis();
void loop(){
  copier.copy();
  if(url.available())
    lastAvailable = millis();
  if(millis()-lastAvailable > 10*1000)
    ESP.restart();
  // pit.processActiolayer.copy();
  // kns();
}

#endif