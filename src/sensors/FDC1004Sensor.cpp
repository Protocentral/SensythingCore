//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Platform - FDC1004 Sensor Adapter Implementation
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

#include "FDC1004Sensor.h"
#include "../core/SensythingConfig.h"

static const ChannelDesc FDC1004_CHANNELS[4] = {
    { "ch0_pf", "CIN1", "pF", -100.0f, 100.0f, CHANNEL_ROLE_WAVEFORM },
    { "ch1_pf", "CIN2", "pF", -100.0f, 100.0f, CHANNEL_ROLE_WAVEFORM },
    { "ch2_pf", "CIN3", "pF", -100.0f, 100.0f, CHANNEL_ROLE_WAVEFORM },
    { "ch3_pf", "CIN4", "pF", -100.0f, 100.0f, CHANNEL_ROLE_WAVEFORM },
};

static const StatusBitDesc FDC1004_STATUS_BITS[1] = {
    { SENSYTHING_STATUS_CAPDAC_ADJ, "capdac_adj", "CAPDAC out of range (adjusting)" },
};

FDC1004Sensor::FDC1004Sensor() {
    capSensor = nullptr;
}

FDC1004Sensor::~FDC1004Sensor() {
    if (capSensor) {
        delete capSensor;
    }
}

const ChannelDesc* FDC1004Sensor::channels() const {
    return FDC1004_CHANNELS;
}

const StatusBitDesc* FDC1004Sensor::statusBits() const {
    return FDC1004_STATUS_BITS;
}

bool FDC1004Sensor::begin(const BoardPins& pins) {
    // Initialize I2C
    Wire.begin(pins.i2cSda, pins.i2cScl);
    Wire.setClock(pins.i2cFreq);

    // Create FDC1004 sensor instance
    if (!capSensor) {
        capSensor = new FDC1004(FDC1004_RATE_100HZ);
    }

    // Initialize sensor
    if (!capSensor->begin()) {
        DEBUG_PRINTLN("FDC1004 begin() failed");
        return false;
    }

    // Verify sensor is responding
    if (!capSensor->isConnected()) {
        DEBUG_PRINTLN("FDC1004 not responding on I2C bus");
        return false;
    }

    DEBUG_PRINTLN("FDC1004 initialized successfully");
    return true;
}

bool FDC1004Sensor::read(MeasurementData& data) {
    if (!capSensor) {
        DEBUG_PRINTLN("Sensor not initialized");
        return false;
    }

    // Initialize measurement data
    data.timestamp = millis();
    data.channel_count = 4;
    data.status_flags = 0;

    // Read all 4 channels
    for (int i = 0; i < 4; i++) {
        fdc1004_capacitance_t measurement = capSensor->getCapacitanceMeasurement(
            static_cast<fdc1004_channel_t>(FDC1004_CHANNEL_0 + i)
        );

        // Check if measurement is valid
        if (!isnan(measurement.capacitance_pf)) {
            data.channels[i] = measurement.capacitance_pf;
            data.metadata[i] = measurement.capdac_used;

            // Check if CAPDAC is adjusting (out of range)
            if (measurement.capdac_out_of_range) {
                data.status_flags |= SENSYTHING_STATUS_CAPDAC_ADJ;
            }
        } else {
            // Measurement failed for this channel
            data.channels[i] = 0.0;
            data.metadata[i] = 0;
            data.status_flags |= (1 << i);  // Set channel fail bit

            DEBUG_PRINTF("Channel %d measurement failed\n", i);
        }
    }

    return true;
}
