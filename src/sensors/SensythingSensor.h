//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Platform - Sensor Adapter Interface
//    One adapter per sensor chip; wraps the chip's public driver library
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

#ifndef SENSYTHING_SENSOR_H
#define SENSYTHING_SENSOR_H

#include <Arduino.h>
#include "../core/SensythingTypes.h"
#include "../core/SensythingConfig.h"

/**
 * A sensor adapter owns one sensor chip. The platform (SensythingCore) talks to the
 * chip only through this interface, so communication modules never need to know
 * which chip a board carries. A board class = SensythingCore + one adapter + BoardPins.
 */
class SensythingSensor {
public:
    virtual ~SensythingSensor() {}

    /**
     * Initialize the sensor on the given board pins
     * @param pins Board pin map
     * @return true if initialization successful, false otherwise
     */
    virtual bool begin(const BoardPins& pins) = 0;

    /**
     * Read one measurement
     * @param data Reference to MeasurementData structure to fill
     * @return true if measurement successful, false otherwise
     */
    virtual bool read(MeasurementData& data) = 0;

    /**
     * Get the sensor description (e.g., "FDC1004 Capacitance")
     */
    virtual const char* sensorType() const = 0;

    /**
     * Get the channel descriptors (channelCount() entries)
     */
    virtual uint8_t channelCount() const = 0;
    virtual const ChannelDesc* channels() const = 0;

    /**
     * Get the sensor-specific status bits (bits 0-3 are the common channel-fail bits)
     */
    virtual uint8_t statusBitCount() const { return 0; }
    virtual const StatusBitDesc* statusBits() const { return nullptr; }

    /**
     * Sample interval limits the sensor supports
     */
    virtual unsigned long minSampleIntervalMs() const = 0;
    virtual unsigned long maxSampleIntervalMs() const = 0;
    virtual unsigned long defaultSampleIntervalMs() const { return SENSYTHING_DEFAULT_SAMPLE_INTERVAL_MS; }
};

#endif // SENSYTHING_SENSOR_H
