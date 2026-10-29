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
    { "ir_raw",     "IR PPG",     "ADC", 0.0f, 524288.0f, CHANNEL_ROLE_WAVEFORM },     // 19-bit ADC
    { "red_raw",    "Red PPG",    "ADC", 0.0f, 524288.0f, CHANNEL_ROLE_WAVEFORM },     // 19-bit ADC
    { "spo2",       "SpO2",       "%",   0.0f, 100.0f,    CHANNEL_ROLE_VITAL_SPO2 },
    { "heart_rate", "Heart rate", "bpm", 0.0f, 250.0f,    CHANNEL_ROLE_VITAL_HR },
};

static const StatusBitDesc AFE4400_STATUS_BITS[3] = {
    { SENSYTHING_STATUS_BUFFER_OVERFLOW, "buffer_overflow", "SpO2 buffer complete" },
    { SENSYTHING_STATUS_NO_SIGNAL,       "no_signal",       "No valid PPG signal detected" },
    { SENSYTHING_STATUS_INVALID_DATA,    "invalid_data",    "Invalid SpO2/HR calculation" },
};

AFE4400Sensor::AFE4400Sensor() {
    ppgSensor = nullptr;
}

AFE4400Sensor::~AFE4400Sensor() {
    if (ppgSensor) {
        delete ppgSensor;
        ppgSensor = nullptr;
    }
}

const ChannelDesc* AFE4400Sensor::channels() const {
    return AFE4400_CHANNELS;
}

const StatusBitDesc* AFE4400Sensor::statusBits() const {
    return AFE4400_STATUS_BITS;
}

bool AFE4400Sensor::begin(const BoardPins& pins) {
    // Initialize SPI
    SPI.begin(pins.spiSck, pins.spiMiso, pins.spiMosi, pins.spiCs);
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.setFrequency(4000000);  // 4 MHz

    // Create AFE44XX sensor instance (published library only takes CS and PWDN pins)
    if (!ppgSensor) {
        ppgSensor = new AFE44XX(pins.spiCs, pins.pwdn);
    }

    // Initialize sensor
    ppgSensor->afe44xx_init();

    DEBUG_PRINTLN("AFE4400 initialized successfully");
    return true;
}

bool AFE4400Sensor::read(MeasurementData& data) {
    if (!ppgSensor) {
        DEBUG_PRINTLN("Sensor not initialized");
        return false;
    }

    // Get data from AFE4400
    if (!ppgSensor->get_AFE44XX_Data(&rawData)) {
        DEBUG_PRINTLN("Failed to read AFE4400 data");
        return false;
    }

    // Initialize measurement data
    data.timestamp = millis();
    data.channel_count = 4;
    data.status_flags = 0;

    // Fill channel data
    // Channel 0: IR raw ADC value
    data.channels[0] = (float)rawData.IR_data;

    // Channel 1: RED raw ADC value
    data.channels[1] = (float)rawData.RED_data;

    // Channel 2: SpO2 percentage (0-100%)
    data.channels[2] = (float)rawData.spo2;

    // Channel 3: Heart rate (bpm)
    data.channels[3] = (float)rawData.heart_rate;

    // Status flags
    if (rawData.buffer_count_overflow) {
        data.status_flags |= SENSYTHING_STATUS_BUFFER_OVERFLOW;
    }

    // Check for invalid heart rate (typically 0 or >250 indicates no signal)
    if (rawData.heart_rate == 0 || rawData.heart_rate > 250) {
        data.status_flags |= SENSYTHING_STATUS_NO_SIGNAL;
    }

    // Check for invalid SpO2 (typically <70% or >100% indicates error)
    if (rawData.spo2 < 70.0 || rawData.spo2 > 100.0) {
        data.status_flags |= SENSYTHING_STATUS_INVALID_DATA;
    }

    // Metadata: Store quality indicators
    data.metadata[0] = (uint8_t)(rawData.spo2);  // SpO2 as byte
    data.metadata[1] = (uint8_t)(rawData.heart_rate);  // HR as byte
    data.metadata[2] = rawData.buffer_count_overflow ? 1 : 0;
    data.metadata[3] = 0;  // Reserved

    return true;
}
