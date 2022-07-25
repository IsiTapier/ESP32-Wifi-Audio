/**
 * @file example-serial-send.ino
 * @author Phil Schatzmann
 * @brief Sending encoded audio over ESPNow
 * @version 0.1
 * @date 2022-03-09
 *
 * @copyright Copyright (c) 2022
 */

#define USE_FDK
#include "AudioTools.h"
#include "AudioLibs/Communication.h"
#include "Arduino.h"
#include "AudioLibs/AudioKit.h"
// #include "AudioCodecs/CodecSBC.h"


uint16_t sample_rate = 32000;
uint8_t channels = 2;  // The stream will have 2 channels
SineWaveGenerator<int16_t> sineWave( 32000);  // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<int16_t> sound( sineWave); // Stream generated from sine wave
AudioKitStream kit;    
AACEncoderFDK *fdk=new AACEncoderFDK();
ESPNowStream now;
EncodedAudioStream encoder(&now, fdk); // encode and write to ESP-now
StreamCopy copier(encoder, kit, 256);  // copies sound into i2s
const char *peers[] = {"A8:48:FA:0B:93:01"};

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);

  LOGLEVEL_AUDIOKIT = AudioKitWarning;
  
  auto cfg = now.defaultConfig();
  cfg.mac_address = "A8:48:FA:0B:93:02";
  now.begin(cfg);
  now.addPeers(peers);

  // fdk = new AACEncoderFDK();  
  fdk->setAudioObjectType(2);  // AAC low complexity
  fdk->setOutputBufferSize(256); // decrease output buffer size
  fdk->setVariableBitrateMode(2); // low variable bitrate

  // Setup sine wave
  auto cfgs = sineWave.defaultConfig();
  cfgs.sample_rate = sample_rate;
  cfgs.channels = channels;
  cfgs.bits_per_sample = 16;
  sineWave.begin(cfgs, N_B4);

  Serial.println("starting AudioKit...");
  auto config = kit.defaultConfig(RX_MODE);
  config.input_device = AUDIO_HAL_ADC_INPUT_LINE2;
  config.sample_rate = 32000; 
  config.bits_per_sample = 16;
  config.channels = 2;
  config.default_actions_active = true;
  config.sd_active = false;
  kit.begin(config);
  Serial.println("AudioKit started");

  // start encoder
  encoder.begin(cfgs);
  
  Serial.println("Sender started...");
}

void loop() { 
  copier.copy();
}