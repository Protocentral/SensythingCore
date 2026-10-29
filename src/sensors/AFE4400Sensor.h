//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Platform - AFE4400 Sensor Adapter
//    PPG / SpO2 / heart rate over SPI (Sensything OX)
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

#ifndef SENSYTHING_AFE4400_SENSOR_H
#define SENSYTHING_AFE4400_SENSOR_H

#include <SPI.h>
#include <protocentral_afe44xx.h>
#include "SensythingSensor.h"

class AFE4400Sensor : public SensythingSensor {
public:
    AFE4400Sensor();
    ~AFE4400Sensor();

    bool begin(const BoardPins& pins) override;
    bool read(MeasurementData& data) override;
    const char* sensorType() const override { return "AFE4400 PPG/SpO2"; }
    uint8_t channelCount() const override { return 4; }
    const ChannelDesc* channels() const override;
    uint8_t statusBitCount() const override { return 3; }
    const StatusBitDesc* statusBits() const override;
    unsigned long minSampleIntervalMs() const override { return 8; }       // 125Hz maximum (8ms period)
    unsigned long maxSampleIntervalMs() const override { return 10000; }   // 0.1Hz minimum
    unsigned long defaultSampleIntervalMs() const override { return 8; }   // SpO2/HR need >= 50Hz reads

private:
    Protocentral_AFE44xx* afe;
    AFE44xx_Algorithms algo;
    uint32_t lastReadUs;
    bool firstRead;
};

#endif // SENSYTHING_AFE4400_SENSOR_H
