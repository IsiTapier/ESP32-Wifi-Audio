/**
 * @file example-serial-receive.ino
 * @author Phil Schatzmann
 * @brief Receiving audio via ESPNow and decoding data to I2S 
 * @version 0.1
 * @date 2022-03-09
 * 
 * @copyright Copyright (c) 2022
 */

#include "AudioTools.h"
#include "AudioLibs/Communication.h"
#include "AudioCodecs/CodecSBC.h"
#include "AudioLibs/AudioKit.h"
#include "Selections.h"

ESPNowStream now;
AudioKitStream kit;
EncodedAudioStream decoder(&kit, new SBCDecoder(256)); // decode and write to I2S - ESP Now is limited to 256 bytes
StreamCopy copier(decoder, now);     
const char *peers[] = {MAC_SENDER};

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);

  // setup esp-now
  auto cfg = now.defaultConfig();
  cfg.mac_address = MAC_RECEIVER1;
  now.begin(cfg);
  now.addPeers(peers);

  Serial.println("starting AudioKit...");
  auto config = kit.defaultConfig(TX_MODE);
  // config.input_device = AUDIO_HAL_ADC_INPUT_LINE2;
  config.output_device = AUDIO_HAL_DAC_OUTPUT_ALL;
  config.sample_rate = 32000; 
  config.bits_per_sample = 16;
  config.channels = 2;
  config.default_actions_active = false;
  config.sd_active = false;
  kit.begin(config);
  Serial.println("AudioKit started");

  decoder.setNotifyAudioChange(now);
  decoder.begin();

  Serial.println("Receiver started...");
}

void loop() { 
  copier.copy();
}