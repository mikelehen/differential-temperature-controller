#ifndef __LOG_H__
#define __LOG_H__

/*
 * Log.h - A configurable logger that sends logs to the cloud and serial out.
 */

#include <assert.h>
#include "CloudStorage.h"

class Log {
  public:
    typedef enum {
      DEBUG = 0,
      INFO,
      WARN,
      ERROR
    } Level;

  private:
    Level _cloudLevel;
    Level _serialLevel;
    CloudStorage _cloud;
  
  public:

    Log() {
      _cloudLevel = Level::INFO;
      _serialLevel = Level::INFO;
    }

    void log(Level level, const String &str) {
      if (level >= _cloudLevel) {
        //_cloud.log(str);
      }
      if (level >= _serialLevel) {
        Serial.println(str);
      }
    }

    void debug(const String &str) {
      log(Level::DEBUG, str);
    }

    void info(const String &str) {
      log(Level::INFO, str);
    }

    void warn(const String &str) {
      log(Level::WARN, str);
    }

    void error(const String &str) {
      log(Level::ERROR, str);
    }

    void setCloudLevel(Level level) {
      _cloudLevel = level;
    }

    void setSerialLevel(Level level) {
      _serialLevel = level;
    }
};

#endif // __LOG_H__
