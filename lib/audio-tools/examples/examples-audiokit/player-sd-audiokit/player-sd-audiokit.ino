/**
 * @file player-sd-audiokit.ino
 * @brief see https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-audiokit/player-sd-audiokit/README.md
 * 
 * @author Phil Schatzmann
 * @copyright GPLv3
 */


#define USE_SDFAT
#include "AudioTools.h"
#include "AudioLibs/AudioKit.h"
#include "AudioCodecs/CodecMP3Helix.h"

const char *startFilePath="/";
const char* ext="mp3";
SdSpiConfig sdcfg(PIN_AUDIO_KIT_SD_CARD_CS, DEDICATED_SPI, SD_SCK_MHZ(10) , &AUDIOKIT_SD_SPI);
AudioSourceSdFat source(startFilePath, ext, sdcfg);
AudioKitStream kit;
MP3DecoderHelix decoder;  // or change to MP3DecoderMAD
AudioPlayer player(source, kit, decoder);

void next(bool, int, void*) {
   player.next();
}

void previous(bool, int, void*) {
   player.previous();
}

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  // setup output
  auto cfg = kit.defaultConfig(TX_MODE);
  kit.begin(cfg);

 // setup additional buttons 
  kit.addAction(PIN_KEY4, next);
  kit.addAction(PIN_KEY3, previous);


  // setup player
  player.setVolume(0.7);
  player.begin();
}

void loop() {
  player.copy();
  kit.processActions();
}
