/**
 * @file example-serial-receive.ino
 * @author Phil Schatzmann
 * @brief Receiving audio via ESPNow
 * @version 0.1
 * @date 2022-03-09
 * 
 * @copyright Copyright (c) 2022
 */

#include "AudioTools.h"
#include "AudioLibs/Communication.h"
#include "AudioLibs/AudioKit.h"
#include "Arduino.h"

ESPNowStream nowIn;
ESPNowStream nowOut;
MeasuringStream now1(nowIn);
AudioKitStream kit;
StreamCopy copier(kit, now1);  // copies sound into i2s
StreamCopy broadcast(nowOut, now1);

const char *inPeers[] = {"A8:48:FA:0B:93:02"};
const char *outPeers[] = {"A8:48:FA:0B:93:02"};

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  Serial.println("starting AudioKit...");
  auto config = kit.defaultConfig(RX_MODE);
  // config.input_device = AUDIO_HAL_ADC_INPUT_LINE2;
  config.output_device = AUDIO_HAL_DAC_OUTPUT_ALL;
  config.sample_rate = 8000; 
  config.bits_per_sample = 16;
  config.channels = 1;
  config.default_actions_active = true;
  config.sd_active = false;
  kit.begin(config);
  Serial.println("AudioKit started");

  auto cfg = nowIn.defaultConfig();
  cfg.mac_address = "A8:48:FA:0B:93:01";
  nowIn.begin(cfg);
  nowIn.addPeers(inPeers);
  nowOut.begin(cfg);
  nowOut.addPeers(outPeers);

  Serial.println("Receiver started...");
}

void loop() { 
  copier.copy();
  broadcast.copy();
}