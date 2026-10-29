//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Platform - AFE4400 Sensor Adapter Implementation
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//   INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//   PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "AFE4400Sensor.h"
#include "../core/SensythingConfig.h"

static const ChannelDesc AFE4400_CHANNELS[4] = {
    { "ir_raw",     "IR PPG",     "ADC", -2097152.0f, 2097151.0f, CHANNEL_ROLE_WAVEFORM },  // signed 22-bit ADC
    { "red_raw",    "Red PPG",    "ADC", -2097152.0f, 2097151.0f, CHANNEL_ROLE_WAVEFORM },  // signed 22-bit ADC
    { "spo2",       "SpO2",       "%",   0.0f, 100.0f,          CHANNEL_ROLE_VITAL_SPO2 },
    { "heart_rate", "Heart rate", "bpm", 0.0f, 250.0f,          CHANNEL_ROLE_VITAL_HR },
};

static const StatusBitDesc AFE4400_STATUS_BITS[3] = {
    { SENSYTHING_STATUS_BUFFER_OVERFLOW, "buffer_overflow", "New SpO2/HR estimate" },
    { SENSYTHING_STATUS_NO_SIGNAL,       "no_signal",       "No valid PPG signal detected" },
    { SENSYTHING_STATUS_INVALID_DATA,    "invalid_data",    "Invalid SpO2/HR calculation" },
};

AFE4400Sensor::AFE4400Sensor() {
    afe = nullptr;
    lastReadUs = 0;
    firstRead = true;
}

AFE4400Sensor::~AFE4400Sensor() {
    if (afe) {
        delete afe;
        afe = nullptr;
    }
}

const ChannelDesc* AFE4400Sensor::channels() const {
    return AFE4400_CHANNELS;
}

const StatusBitDesc* AFE4400Sensor::statusBits() const {
    return AFE4400_STATUS_BITS;
}

bool AFE4400Sensor::begin(const BoardPins& pins) {
    if (!afe) {
        afe = new Protocentral_AFE44xx(pins.spiCs, pins.drdy, pins.pwdn, pins.reset);
    }
    
    // The AFE4400 and AFE4490 share a register map but not every bit, and
    // there is no ID register, so the driver has to be told which chip it is.
    AFE44xx_Config config;
    config.chip = AFE44xx_Chip::AFE4400;
    
    // Starts SPI on the board pins, resets the chip and checks SPI read-back
    if (!afe->begin(pins.spiSck, pins.spiMiso, pins.spiMosi, config)) {
        DEBUG_PRINTF("AFE4400 begin() failed, error %d\n", (int)afe->lastError());
        return false;
    }
    
    algo.reset();
    firstRead = true;
    
    DEBUG_PRINTLN("AFE4400 initialized successfully");
    return true;
}

bool AFE4400Sensor::read(MeasurementData& data) {
    if (!afe) {
        DEBUG_PRINTLN("Sensor not initialized");
        return false;
    }
    
    // Read the latest sample from all six result registers
    AFE44xx_Sample sample;
    if (!afe->readLatest(sample)) {
        DEBUG_PRINTLN("Failed to read AFE4400 data");
        return false;
    }
    
    // Reads are paced by the platform, not the AFE: time them and let the
    // algorithms resample to their design rate (needs >= 50 reads/s)
    uint32_t now = micros();
    bool updated = false;
    if (firstRead) {
        firstRead = false;
    } else {
        updated = algo.processSampleTimed(sample.ir(), sample.red(), now - lastReadUs);
    }
    lastReadUs = now;
    
    // Initialize measurement data
    data.timestamp = millis();
    data.channel_count = 4;
    data.status_flags = 0;
    
    // Fill channel data
    // Channel 0: IR raw ADC value (LED1)
    data.channels[0] = (float)sample.led1;
    
    // Channel 1: RED raw ADC value (LED2)
    data.channels[1] = (float)sample.led2;
    
    // Channel 2: SpO2 percentage (0 when invalid)
    data.channels[2] = (float)algo.spo2();
    
    // Channel 3: Heart rate (bpm, 0 when invalid)
    data.channels[3] = (float)algo.heartRate();
    
    // Status flags
    if (updated) {
        data.status_flags |= SENSYTHING_STATUS_BUFFER_OVERFLOW;
    }
    
    if (!algo.heartRateValid()) {
        data.status_flags |= SENSYTHING_STATUS_NO_SIGNAL;
    }
    
    if (!algo.spo2Valid()) {
        data.status_flags |= SENSYTHING_STATUS_INVALID_DATA;
    }
    
    // Metadata: Store quality indicators
    data.metadata[0] = algo.spo2();
    data.metadata[1] = algo.heartRate();
    data.metadata[2] = updated ? 1 : 0;
    data.metadata[3] = algo.signalQuality();  // AFE44XX_QUALITY_* flags, 0 = good
    
    return true;
}
