#ifndef __CLOUD_STORAGE_H__
#define __CLOUD_STORAGE_H__

#include <Firebase.h>
#include <FirebaseArduino.h>
#include <FirebaseCloudMessaging.h>
#include <FirebaseError.h>
#include <FirebaseHttpClient.h>
#include <FirebaseObject.h>

class CloudStorage {
  private:
    // We store as much of the configuration as possible in the cloud so that we can change
    // these parameters without reflashing the device.  The values below are overwritten by
    // the value stored the specified path in the Firebase database (if any).

    const char* const _config_ref                   = "config";

    // The fixed resistance of the resistor in the voltage divider (in ohms).
    const char* const _series_resistor_ref          = "seriesResistor";
    float   _series_resistor                        = 8170;

    // The resistance of the thermistor (in ohms) at a known temperature.
    const char* const _resistance_at_0_ref          = "resistanceAt0";
    float   _resistance_at_0                        = 9555.55;
    
    // The temperature at which the resistance of the thermistor was measured.
    const char* const _temperature_at_0_ref         = "temperatureAt0";
    float   _temperature_at_0                       = 25;

    // The calculated b-coefficient of the thermistor in the Steinhart-Hart equation.
    const char* const _b_coefficient_ref            = "bCoefficient";
    float   _b_coefficient                          = 3380;

    // The frequency at which we make a decision about engaging/disengaging the solar
    // collector, and at which we log temperature data to the Firebase database.
    const char* const _polling_milliseconds_ref     = "pollingMilliseconds";
    int     _polling_milliseconds                   = 5 * 1000;

    // The maximum number of temperature sample points we store in the Firebase database.
    const char* const _max_entries_ref              = "maxEntries";
    int     _max_entries                            = 0;

    // The NTP server used to synchronize the 'Time' library.
    const char* const _ntp_server_ref               = "ntpServer";
    String  _ntp_server                             = "pool.ntp.org";

    // The GMT offset.  Only used when logging data to the serial monitor.
    const char* const _gmt_offset_ref               = "gmtOffset";
    int     _gmt_offset                             = 0;

    // The minimum absolute temperature required to engage the solar collector.  (Used
    // to prevent engaging the collector during near freezing conditions.)
    const char* const _min_t_on_ref               = "minTOn";
    float   _min_t_on                             = 10;

    // The minimum temperature delta required to engage the solar collector.
    const char* const _delta_t_on_ref               = "deltaTOn";
    float   _delta_t_on                             = 10;

    // The delta at which we will disengage the solar collector.
    const char* const _delta_t_off_ref             = "deltaTOff";
    float   _delta_t_off                            = 0;

    // The number of temperature sample points taken and averaged between each iteration
    // of the polling loop.
    const char* const _oversample_ref               = "oversample";
    int     _oversample                             = 16;

    // Path to here datapoints are logged in the Firebase database.
    const String _log_ref                           = String("log");

    // The current log entry (wraps at '_max_entries'.)
    uint32_t _current_entry                         = 0;

    // Convenience method used to log 'FAILED' and return false (leading to an early exit)
    // when 'Firebase.failed()' returns true.
    bool failed() {
      if (Firebase.failed()) {
        Serial.println("[FAILED]");
        return true;
      }
      
      return false;
    }
    
    // Template used by 'Firebase_maybeUpdate*()' (below) to update the 'value' with the
    // value stored at 'path', if we're able to successfully retrieve it.  Otherwise, return
    // false and leave 'value' unmodified.
    //
    // This was used during development to fallback on built-in default values before
    // the Firebase database was populated.
    template <typename T> bool maybeUpdate(T (*getFn)(FirebaseObject& obj, const String& path), FirebaseObject& obj, const char* const path, T& value) {
      Serial.print("  Accessing '"); Serial.print(path); Serial.print("': ");
      T maybeNewValue = getFn(obj, path);
      if (failed()) {
        return false;
      }

      value = maybeNewValue;
      Serial.println(value);
      return true;
    };

    // Thunks w/known addresses so we can create function pointers.
    static int Firebase_getInt(FirebaseObject& obj, const String& path) { return obj.getInt(path); }
    static float Firebase_getFloat(FirebaseObject& obj, const String& path) { return obj.getFloat(path); }
    static String Firebase_getString(FirebaseObject& obj, const String& path) { return obj.getString(path); }

    // Updates 'value' with the Firebase value at 'path', if any.  Otherwise leaves
    // 'value' unmodified and returns false.
    bool maybeUpdateInt(FirebaseObject& obj, const char* const path, int& value) {
      return maybeUpdate<int>(Firebase_getInt, obj, path, value);
    }
    
    // Updates 'value' with the Firebase value at 'path', if any.  Otherwise leaves
    // 'value' unmodified and returns false.
    bool maybeUpdateFloat(FirebaseObject& obj, const char* const path, float& value) {
      return maybeUpdate<float>(Firebase_getFloat, obj, path, value);
    }

    // Updates 'value' with the Firebase value at 'path', if any.  Otherwise leaves
    // 'value' unmodified and returns false.
    bool maybeUpdateString(FirebaseObject& obj, const char* const path, String& value) {
      return maybeUpdate<String>(Firebase_getString, obj, path, value);
    }

  public:
    // Updates cached configuration with values from Firebase.
    bool update(Device& device) {
      bool success = true;

      Serial.print("Updating config from Firebase: ");
      device.blinkLed(25);

      FirebaseObject configObj = Firebase.get(_config_ref);
      if (configObj.failed()) {
        return false;
      }
      
      configObj.getJsonVariant().printTo(Serial); Serial.println();

      success &= maybeUpdateFloat(configObj, _series_resistor_ref, _series_resistor);
      success &= maybeUpdateFloat(configObj, _temperature_at_0_ref, _temperature_at_0);
      success &= maybeUpdateFloat(configObj, _resistance_at_0_ref, _resistance_at_0);
      success &= maybeUpdateFloat(configObj, _b_coefficient_ref, _b_coefficient);
      success &= maybeUpdateInt(configObj,_polling_milliseconds_ref, _polling_milliseconds);
      success &= maybeUpdateInt(configObj, _max_entries_ref, _max_entries);
      success &= maybeUpdateString(configObj, _ntp_server_ref, _ntp_server);
      success &= maybeUpdateInt(configObj, _gmt_offset_ref, _gmt_offset);
      success &= maybeUpdateFloat(configObj, _delta_t_on_ref, _delta_t_on);
      success &= maybeUpdateFloat(configObj, _delta_t_off_ref, _delta_t_off);
      success &= maybeUpdateFloat(configObj, _min_t_on_ref, _min_t_on);
      success &= maybeUpdateInt(configObj, _oversample_ref, _oversample);
      device.setLed(true);

      return success;
    }

    // Initializes connection to Firebase database.
    bool init(const String& firebase_host, const String& firebase_auth) {
      Serial.print("Conecting to Firebase '"); Serial.print(firebase_host); Serial.print("': ");

      Firebase.begin(firebase_host, firebase_auth);
      if (!failed()) {
        Serial.println("[OK]");
      }
    }

    // The frequency at which we make a decision about engaging/disengaging the solar
    // collector, and at which we log temperature data to the Firebase database.
    int getPollingMilliseconds() const {
      return _polling_milliseconds;
    }

    // The fixed resistance of the resistor in the voltage divider (in ohms).
    double getSeriesResistor() const {
      return _series_resistor;
    }

    double getResistanceAt0() const {
      return _resistance_at_0;
    }

    double getTemperatureAt0() const {
      return _temperature_at_0;
    }

    double getBCoefficient() const {
      return _b_coefficient;
    }

    double getMinTOn() const {
      return _min_t_on;
    }

    double getDeltaTOn() const {
      return _delta_t_on;
    }

    double getDeltaTOff() const {
      return _delta_t_off;
    }
    
    double getOversample() const {
      return _oversample;
    }
    
    const char* const getNtpServer() const {
      return _ntp_server.c_str();
    }

    int8_t getGmtOffset() const {
      assert(-11 <= _gmt_offset && _gmt_offset <= 13);
      
      return static_cast<int8_t>(_gmt_offset);
    }

    void pushLogInt(String name, int value) {
      String slotRef = _log_ref + "/" + name + "/" + _current_entry;

      Serial.print("  Logging '"); Serial.print(slotRef); Serial.print("': ");

      bool fail = true;
      for (int i = 0; fail && i < 3; i++) {
        Firebase.setInt(slotRef, value);
        fail = failed();
        if (fail) {
          Serial.print(".");
        }
      }

      if (!fail) {
        Serial.println(value);
      }
    }

    void log(Device& device, time_t timestamp, double adc0, double adc1, bool active) {
      device.blinkLed(19);

      DynamicJsonBuffer _json_buffer;
      JsonObject& root = _json_buffer.createObject();
      root["time"] = timestamp;
      root["0"] = adc0;
      root["1"] = adc1;
      root["active"] = active;
      
      String slotRef = _log_ref + "/" + _current_entry;

      Serial.print("  Logging '"); Serial.print(slotRef); Serial.print("': ");

      bool fail = true;
      for (int i = 0; fail && i < 3; i++) {
        Firebase.set(slotRef, root);
        fail = failed();
        if (fail) {
          Serial.print("  ... ");
        }
        delay(100);
      }

      if (!fail) {
        root.printTo(Serial); Serial.println();
        _current_entry = (_current_entry + 1) % _max_entries;
      }

      device.setLed(true);
    }
};

#endif // __CLOUD_STORAGE_H__
