//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Platform - FDC1004 Sensor Adapter
//    4-channel capacitance measurement over I2C (Sensything Cap)
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

#ifndef SENSYTHING_FDC1004_SENSOR_H
#define SENSYTHING_FDC1004_SENSOR_H

#include <Wire.h>
#include <Protocentral_FDC1004.h>
#include "SensythingSensor.h"
#include "../core/SensythingConfig.h"

class FDC1004Sensor : public SensythingSensor {
public:
    FDC1004Sensor();
    ~FDC1004Sensor();

    bool begin(const BoardPins& pins) override;
    bool read(MeasurementData& data) override;
    const char* sensorType() const override { return "FDC1004 Capacitance"; }
    uint8_t channelCount() const override { return 4; }
    const ChannelDesc* channels() const override;
    uint8_t statusBitCount() const override { return 1; }
    const StatusBitDesc* statusBits() const override;
    unsigned long minSampleIntervalMs() const override { return SENSYTHING_MIN_SAMPLE_INTERVAL_MS; }
    unsigned long maxSampleIntervalMs() const override { return SENSYTHING_MAX_SAMPLE_INTERVAL_MS; }

private:
    FDC1004* capSensor;
};

#endif // SENSYTHING_FDC1004_SENSOR_H
