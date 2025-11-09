//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    Sensything Platform - Main Library Header
//////////////////////////////////////////////////////////////////////////////////////////
//    (c) 2025 Protocentral Electronics
//
//    SensythingES3 - Main Library Header
//    Unified Arduino library for SensythingES3 ESP32-S3 sensor boards
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

#ifndef SENSYTHING_ES3_H
#define SENSYTHING_ES3_H

#include "core/SensythingConfig.h"
#include "core/SensythingTypes.h"
#include "core/SensythingCore.h"
#include "boards/SensythingCap.h"
#include "boards/SensythingOX.h"

#define SENSYTHING_ES3_VERSION SENSYTHING_ES3_NAME

#endif // SENSYTHING_ES3_H
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

#ifndef SENSYTHING_H
#define SENSYTHING_H

// Core platform files
#include "core/SensythingTypes.h"
#include "core/SensythingConfig.h"
#include "core/SensythingCore.h"

// Board implementations
#include "boards/SensythingCap.h"
#include "boards/SensythingOX.h"

// Communication modules
#include "communication/SensythingUSB.h"
#include "communication/SensythingBLE.h"
#include "communication/SensythingWiFi.h"
#include "communication/SensythingSDCard.h"
#include "communication/SensythingBLE.h"
#include "communication/SensythingSDCard.h"

// Version information
#define SENSYTHING_VERSION SENSYTHING_PLATFORM_VERSION
#define SENSYTHING_VERSION_MAJOR 1
#define SENSYTHING_VERSION_MINOR 0
#define SENSYTHING_VERSION_PATCH 0

#endif // SENSYTHING_H
