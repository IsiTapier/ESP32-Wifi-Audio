#pragma once
#include "Arduino.h"
#include "AudioConfig.h"
#include "AudioTools/AudioTypes.h"
#include "AudioTools/Converter.h"
#include "AudioTools/Buffers.h"
#include "AudioTools/AudioStreams.h"
#include "AudioBasic/Int24.h"
#include "WiFiClient.h"
#include "vector"

#define MAX_SINGLE_CHARS 8

namespace audio_tools {

/**
 * @brief Abstract Audio Ouptut class
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class AudioPrint : public Print, public AudioBaseInfoDependent, public AudioBaseInfoSource {
    public:
        virtual size_t write(const uint8_t *buffer, size_t size) override = 0;

        virtual size_t write(uint8_t ch) override {
            tmp[tmpPos++] = ch;
            if (tmpPos>MAX_SINGLE_CHARS){
                flush();
            } 
            return 1;
        }

        void flush() FLUSH_OVERRIDE {
            write((const uint8_t*)tmp, tmpPos-1);
            tmpPos=0;
        }

        // overwrite to do something useful
        virtual void setAudioInfo(AudioBaseInfo info) {
            LOGD(LOG_METHOD);
            cfg = info;
            info.logInfo();
            if (p_notify!=nullptr){
                p_notify->setAudioInfo(info);
            }
        }

        virtual void  setNotifyAudioChange(AudioBaseInfoDependent &bi) {
            p_notify = &bi;
        }

        /// If true we need to release the related memory
        virtual bool doRelease() {
            return false;
        }

        virtual AudioBaseInfo audioInfo() {
            return cfg;
        }

    protected:
        uint8_t tmp[MAX_SINGLE_CHARS];
        int tmpPos=0;
        AudioBaseInfoDependent *p_notify=nullptr;
        AudioBaseInfo cfg;

};



/**
 * @brief Stream Wrapper which can be used to print the values as readable ASCII to the screen to be analyzed in the Serial Plotter
 * The frames are separated by a new line. The channels in one frame are separated by a ,
 * @tparam T 
 * @author Phil Schatzmann
 * @copyright GPLv3
*/
template<typename T>
class CsvStream : public AudioPrint {

    public:
        CsvStream(int buffer_size=DEFAULT_BUFFER_SIZE, bool active=true) {
            this->active = active;
        }

        /// Constructor
        CsvStream(Print &out, int channels=2, int buffer_size=DEFAULT_BUFFER_SIZE, bool active=true) {
            this->channels = channels;
            this->out_ptr = &out;
            this->active = active;
        }

        /// Starts the processing with 2 channels
        void begin(){
             LOGD(LOG_METHOD);
            this->active = true;
        }

        /// Provides the default configuration
        AudioBaseInfo defaultConfig(){
            AudioBaseInfo info;
            info.channels = 2;
            info.sample_rate = 44100;
            info.bits_per_sample = sizeof(T)*8;
            return info;
        }

        /// Starts the processing with the number of channels defined in AudioBaseInfo
        void begin(AudioBaseInfo info){
             LOGD(LOG_METHOD);
            this->active = true;
            this->channels = info.channels;
        }

        /// Starts the processing with the defined number of channels 
        void begin(int channels, Print &out=Serial){
             LOGD(LOG_METHOD);
            this->channels = channels;
            this->out_ptr = &out;
            this->active = true;
        }

        /// Sets the CsvStream as inactive 
        void end() {
             LOGD(LOG_METHOD);
            active = false;
        }

        /// defines the number of channels
        virtual void setAudioInfo(AudioBaseInfo info) {
             LOGI(LOG_METHOD);
            info.logInfo();
            this->channels = info.channels;
        };

        /// Writes the data - formatted as CSV -  to the output stream
        virtual size_t write(const uint8_t* data, size_t len) {   
            if (!active) return 0;
            LOGD(LOG_METHOD);
            size_t lenChannels = len / (sizeof(T)*channels); 
            data_ptr = (T*)data;
            for (size_t j=0;j<lenChannels;j++){
                for (int ch=0;ch<channels;ch++){
                    if (out_ptr!=nullptr && data_ptr!=nullptr){
                        out_ptr->print(*data_ptr);
                    }
                    data_ptr++;
                    if (ch<channels-1) Serial.print(", ");
                }
                Serial.println();
            }
            return len;
        }

    protected:
        T *data_ptr;
        Print *out_ptr = &Serial;
        int channels = 2;
        bool active = false;

};

/**
 * @brief Creates a Hex Dump
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class HexDumpStream : public AudioPrint {

    public:
        HexDumpStream(int buffer_size=DEFAULT_BUFFER_SIZE, bool active=true) {
            this->active = active;
        }

        /// Constructor
        HexDumpStream(Print &out, int buffer_size=DEFAULT_BUFFER_SIZE, bool active=true) {
            this->out_ptr = &out;
            this->active = active;
        }

        void begin(AudioBaseInfo info){
            LOGD(LOG_METHOD);
            info.logInfo();
            this->active = true;
            pos = 0;
        }

        void begin(){
             LOGD(LOG_METHOD);
            this->active = true;
            pos = 0;
        }

        /// Sets the CsvStream as inactive 
        void end() {
             LOGD(LOG_METHOD);
            active = false;
        }

        void flush(){
            Serial.println();
            pos = 0;
        }

        virtual size_t write(const uint8_t* data, size_t len) {   
            if (!active) return 0;
             LOGD(LOG_METHOD);
            for (size_t j=0;j<len;j++){
                out_ptr->print(data[j], HEX);
                out_ptr->print(" ");
                pos++;
                if (pos == 8){
                    Serial.print(" - ");
                }
                if (pos == 16){
                    Serial.println();
                    pos = 0;
                }
            }
            return len;
        }

    protected:
        Print *out_ptr = &Serial;
        int pos = 0;
        bool active = false;
};


/**
 * @brief Wrapper which converts a AudioStream to a AudioPrint
 * 
 */
class AdapterAudioStreamToAudioPrint : public AudioPrint {
    public: 
        AdapterAudioStreamToAudioPrint(AudioStream &stream){
            p_stream = &stream;
        }
        void setAudioInfo(AudioBaseInfo info){
            p_stream->setAudioInfo(info);
        }
        size_t write(const uint8_t *buffer, size_t size){
            return p_stream->write(buffer,size);
        }

        virtual bool doRelease() {
            return true;
        }
       
    protected:
        AudioStream *p_stream=nullptr;
};

/**
 * @brief Wrapper which converts a Print to a AudioPrint
 * 
 */
class AdapterPrintToAudioPrint : public AudioPrint {
    public: 
        AdapterPrintToAudioPrint(Print &print){
            p_print = &print;
        }
        void setAudioInfo(AudioBaseInfo info){
        }
        size_t write(const uint8_t *buffer, size_t size){
            return p_print->write(buffer,size);
        }
        virtual bool doRelease() {
            return true;
        }
    protected:
        Print *p_print=nullptr;
};


/**
 * @brief Replicates the output to multiple destinations.
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class MultiOutput : public AudioPrint {
    public:

        /// Defines a MultiOutput with no final output: Define your outputs with add()
        MultiOutput() = default;

        /// Defines a MultiOutput with a single final outputs,
        MultiOutput(AudioPrint &out){
            add(out);            
        }

        MultiOutput(AudioStream &out){
            add(out);            
        }

        /// Defines a MultiOutput with 2 final outputs
        MultiOutput(AudioPrint &out1, AudioPrint &out2){
            add(out1);
            add(out2);
        }

        /// Defines a MultiOutput with 2 final outputs
        MultiOutput(AudioStream &out1, AudioStream &out2){
            add(out1);
            add(out2);
        }

        ~MultiOutput() {
            for (int j=0;j<vector.size();j++){
                if (vector[j]->doRelease()){
                    delete vector[j];
                }
            }
        }

        bool begin(AudioBaseInfo info){
            setAudioInfo(info);
            return true;
        }

        // void setOuts(Vector<AudioPrint> &outs) {
        //     vector.clear();
        //     for(int i = 0; i < outs.size(); i++)  {
        //         vector.push_back(&outs[i]);
        //     }
        // }

        // void setOuts(Vector<AudioStream> &outs) {
        //     vector.clear();
        //     for(int i = 0; i < outs.size(); i++)  {
        //         AudioPrint* out = new AdapterAudioStreamToAudioPrint(outs[i]);
        //         vector.push_back(out);
        //     }
        // }

        // void setOuts(Vector<Print> &outs) {
        //     vector.clear();
        //     for(int i = 0; i < outs.size(); i++)  {
        //         AudioPrint* out = new AdapterPrintToAudioPrint(outs[i]);
        //         vector.push_back(out);
        //     }
        // }

        // void setOuts(std::vector<WiFiClient> &outs) {
        //     vector.clear();
        //     for(int i = 0; i < outs.size(); i++)  {
        //         AudioPrint* out = new AdapterPrintToAudioPrint(outs[i]);
        //         vector.push_back(out);
        //     }
        // }

        void clear() {
            vector.clear();
        }

        /// Add an additional AudioPrint output
        void add(AudioPrint &out){
            vector.push_back(&out);
        }

        /// Add an AudioStream to the output
        void add(AudioStream &stream){
            AdapterAudioStreamToAudioPrint* out = new AdapterAudioStreamToAudioPrint(stream);
            vector.push_back(out);
        }

        void add(Print &print){
            AdapterPrintToAudioPrint* out = new AdapterPrintToAudioPrint(print);
            vector.push_back(out);
        }

        void flush() {
            for (int j=0;j<vector.size();j++){
                vector[j]->flush();
            }
        }

        void setAudioInfo(AudioBaseInfo info){
            for (int j=0;j<vector.size();j++){
                vector[j]->setAudioInfo(info);
            }
        }

        size_t write(const uint8_t *buffer, size_t size){
            for (int j=0;j<vector.size();j++){
                int open = size;
                int start = 0;
                while(open>0){
                    int written = vector[j]->write(buffer+start, open);
                    open -= written;
                    start += written;
                }
            }
            return size;
        }

        size_t write(uint8_t ch){
            for (int j=0;j<vector.size();j++){
                int open = 1;
                while(open>0){
                    open -= vector[j]->write(ch);
                }
            }
            return 1;
        }

    protected:
        Vector<AudioPrint*> vector;

};


#include "Arduino.h"

class MultiOutputPrint : public AudioPrint {
    public:

        /// Defines a MultiOutput with no final output: Define your outputs with add()
        MultiOutputPrint() = default;

        MultiOutputPrint(std::vector<WiFiClient>* vec) {
            vector = vec;
        }

        /// Defines a MultiOutput with a single final outputs,
        // MultiOutputPrint(Print &out){
        //     vector.push_back(&out);            
        // }

        // /// Defines a MultiOutput with 2 final outputs
        // MultiOutputPrint(Print &out1, Print &out2){
        //     vector.push_back(&out1);
        //     vector.push_back(&out2);
        // }

        // /// Add an additional AudioPrint output
        // void add(Print &out){
        //     vector.push_back(&out);
        // }

        // void remove(Print* out) {
        //     Serial.println("test");
        //     for(auto obj = vector.begin(); obj != vector.end(); obj++) {
        //         Serial.println("test2");
        //         if(*obj == out) {
        //             vector.erase(obj);
        //             return;
        //         }
        //     }
        // }

        // void clear() {
        //     vector.clear();
        // }
        // void flush() {
        //     for (int j=0;j<vector.size();j++){
        //         // vector[j]->flush();
        //     }
        // }

        // void setAudioInfo(AudioBaseInfo info){
        //     for (int j=0;j<vector.size();j++){
        //         // vector[j]->setAudioInfo(info);
        //     }
        // }

        size_t write(const uint8_t *buffer, size_t size){
            for (int j=0;j<vector->size();j++){
                int open = size;
                int start = 0;
                while(open>0){
                    int written = (*vector)[j].write(buffer+start, open);
                    open -= written;
                    start += written;
                    if(written==0) (*vector)[j].stop();
                    if((*vector)[j].getWriteError()!=0 || !(*vector)[j].connected() || !(*vector)[j] || written == 0) break;
                }
            }
            return size;
        }

        size_t write(uint8_t ch){
            for (int j=0;j<vector->size();j++){
                int open = 1;
                while(open>0){
                    open -= (*vector)[j].write(ch);
                    if((*vector)[j].getWriteError()!=0 || !(*vector)[j].connected() || !(*vector)[j])
                        break;
                }
                // (*vector)[j].write(ch);
            }
            return 1;
        }

    protected:
        std::vector<WiFiClient>* vector;

};

/**
 * @brief Mixing of multiple outputs to one final output
 * @author Phil Schatzmann
 * @copyright GPLv3
 * @tparam T 
 */
template<typename T>
class OutputMixer : public Print {
  public:
    OutputMixer(Print &finalOutput, int outputStreamCount) {
      p_final_output = &finalOutput;
      output_count = outputStreamCount;
      for (int i=0;i<output_count;i++){
          weights.push_back(1.0);
      }
    };

    /// Defines a new weight for the indicated channel: If you set it to 0 it is muted.
    void setWeight(int channel, float weight){
      if (channel<size()){
        weights[channel] = weight;
      } else {
        LOGE("Invalid channel %d - max is %d", channel, size()-1);
      }
    }

    void begin(int copy_buffer_size=DEFAULT_BUFFER_SIZE) {
      is_active = true;
      total_weights = 0;
      for (int j=0;j<weights.size();j++){
        total_weights += weights[j];
      }
      size_bytes = copy_buffer_size;
      // clear final data so that we can add values
      result.resize(size_bytes);
      memset(result.data(), 0, size_bytes); 
    }

    /// Remove all input streams
    void end() {
      weights.clear();
      total_weights = 0.0;
      is_active = false;
    }

    /// Number of stremams to which are mixed together
    int size() {
      return output_count;
    }

    /// Write the data from multiple streams mixed together 
    size_t write(const uint8_t *buffer_c, size_t size){
        if (!is_active) return 0;
        LOGD("write: %d", size);
        int sample_size = min(size, (size_t)size_bytes);
        int sample_count = sample_size / sizeof(T);
        T* samples_result = (T*)result.data();
        float weight = weights[stream_idx];
        // sum up input samples to result samples 
        T *buffer = (T*) buffer_c;
        for (int i=0;i<sample_count; i++){
            result[i] += weight * buffer[i] / total_weights;
        }
        stream_idx++;
        if (stream_idx>=output_count){
            flush();
        }
        return sample_size;
    }
    
    int availableForWrite() { return is_active ? size_bytes : 0; }

    /// Force output to final destination
    void flush() {
        p_final_output->write(result.data(), size_bytes);
        stream_idx = 0;
        // clear final data so that we can add values
        memset(result.data(), 0, size_bytes); 
    }

  protected:
    Vector<uint8_t> result{0};
    Vector<float> weights{10}; 
    Print *p_final_output=nullptr;
    float total_weights = 0.0;
    bool is_active = false;
    int stream_idx = 0;
    int size_bytes;
    int output_count;

};


/**
 * @brief A simple class to determine the volume
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class VolumePrint : public AudioPrint {
    public:
        VolumePrint() = default;

        ~VolumePrint() {
            if (volumes!=nullptr) delete volumes;
            if (volumes_tmp!=nullptr) delete volumes_tmp;
        }
     
        bool begin(AudioBaseInfo info){
            setAudioInfo(info);
            return true;
        }

        void setAudioInfo(AudioBaseInfo info){
            this->info = info;
            if (info.channels){
                volumes = new float[info.channels];
                volumes_tmp = new float[info.channels];
            }
        }

        size_t write(const uint8_t *buffer, size_t size){
            float f_volume_tmp = 0;
            for (int j=0;j<info.channels;j++){
                volumes_tmp[j]=0;
            }
            switch(info.bits_per_sample){
                case 16: {
                        int16_t *buffer16 = (int16_t*)buffer;
                        int samples16 = size/2;
                        for (int j=0;j<samples16;j++){
                            float tmp = static_cast<float>(abs(buffer16[j]));
                            updateVolume(tmp,j);
                        }
                        commit();
                    } break;
                case 32: {
                        int32_t *buffer32 = (int32_t*)buffer;
                        int samples32 = size/4;
                        for (int j=0;j<samples32;j++){
                            float tmp = static_cast<float>(abs(buffer32[j]))/samples32;
                            updateVolume(tmp,j);
                        }
                        commit();
                    }break;

                default:
                    LOGE("Unsupported bits_per_sample: %d", info.bits_per_sample);
                    f_volume = 0;
                    break;
            }
            return size;
        }

        /// Determines the volume (the range depends on the bits_per_sample)
        float volume() {
            return f_volume;
        }

        /// Determines the volume for the indicated channel
        float volume(int channel) {
            return channel<info.channels ? volumes[channel]:0.0;
        }

    protected:
        AudioBaseInfo info;
        float f_volume_tmp = 0;
        float f_volume = 0;
        float *volumes=nullptr;
        float *volumes_tmp=nullptr;

        void updateVolume(float tmp, int j) {
            if (tmp>f_volume){
                f_volume_tmp = tmp;
            }
            if (volumes_tmp!=nullptr && tmp>volumes_tmp[j%info.channels]){
                volumes_tmp[j%info.channels] = tmp;
            }
        }

        void commit(){
            f_volume = f_volume_tmp;
            for (int j=0;j<info.channels;j++){
                volumes[j] = volumes_tmp[j];
            }
        }
};


} //n namespace