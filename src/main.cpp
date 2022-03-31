// #define USE_HELIX 
#define USE_FDK

#include "AudioTools.h"
#include "AudioLibs/AudioKit.h"
// #include "Arduino.h

// example audio
const char* urls[] = {
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-1.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-2.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-3.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-4.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-5.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-6.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-7.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-8.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-9.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-10.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-11.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-12.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-13.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-14.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-15.mp3",
  "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-16.mp3"
};

// Wifi settings
const char* ssid = "JCBS-Schüler";
const char* password = "S1,16DismdEn;deieKG!";
// const char* ssid = "AndroidAP4C1B";
// const char* password = "12345678";
// const char* ssid = "NetFrame";
// const char* password = "87934hzft9oeu4389nv8o437893hf978";
// const char* ssid = "PSM_Gast";
// const char* password = "2016Daheim12";
// const char* ssid = "SSID";
// const char* password = "PASSWORD";,

AudioKitStream kit;
// ICYStream urlStream(ssid, password);
// AudioSourceURL source(urlStream, urls, "audio/mp3");
// MP3DecoderHelix decoder;
// AudioPlayer player(source, kit, decoder);

AACEncoderFDK *fdk=nullptr;
AudioEncoderServer *server=nullptr; 
// AudioEncoderServer server(new AACEncoderFDK(), ssid, password); 

bool playState = true;
bool mode = true;

void next(bool, int, void*) {
  if(!mode)
    // player.next();
  Serial.println("next");
}

void previous(bool, int, void*) {
  if(!mode)
    // player.previous();
  Serial.println("pevious");
}

#define BLA 10

void changeMode(bool, int, void*) {
   mode = !mode;
   if(mode) {
      // player.stop();
   } else {
      // player.play();
   }
   Serial.println("Mode changed to "+String(mode?"send":"receive"));
}


void setup(){

  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);
  Serial.println("Serial started");

  Serial.println("starting AudioKit...");
  auto config = kit.defaultConfig(RXTX_MODE);
  config.input_device = AUDIO_HAL_ADC_INPUT_LINE2;
  config.bits_per_sample = 16;
  config.sample_rate = 44100;
  config.channels = 2;
  config.default_actions_active = false;
  config.sd_active = false;
  kit.begin(config);
  Serial.println("AudioKit started");

  fdk = new AACEncoderFDK();  
  fdk->setAudioObjectType(2);  // AAC low complexity
  fdk->setOutputBufferSize(1024); // decrease output buffer size
  fdk->setVariableBitrateMode(2); // low variable bitrate
  server = new AudioEncoderServer(fdk,ssid,password);  
  server->begin(kit, config);
  
  // player.setVolume(1.);
  // player.begin();
  // player.stop();
 

  // kit.addAction(PIN_KEY4, next);
  // kit.addAction(PIN_KEY3, previous);
  // kit.addAction(PIN_KEY2, changeMode);

  // auto down = [](bool,int,void*) { if(!mode) AudioKitStream::actionVolumeDown(true, -1, nullptr); Serial.println("Volume down"); };
  // kit.addAction(PIN_KEY5, down);
  // auto up = [](bool,int,void*) { if(!mode) AudioKitStream::actionVolumeUp(true, -1, nullptr ); Serial.println("Volume up"); };
  // kit.addAction(PIN_KEY6, up);
  // auto play = [](bool,int,void*) { AudioKitStream::actionStartStop(true, -1, nullptr ); playState=!playState; Serial.println(playState?"Continue playing" : "Stop playing"); };
  // kit.addAction(PIN_KEY1, play);
}

void loop() {
  if(mode)
    server->doLoop();
  // else
    // player.copy();
  // kit.processActions();
}