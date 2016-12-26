#ifndef __THERMISTOR_H__
#define __THERMISTOR_H__

class ThermistorReading {
  public:
    const double _adc;                        // Raw ADC reading [0..1023]
    const double _resistance;                 // Corresponding thermistor resistance (in Ohms)
    const double _celsius;                    // Corresponding temperature (in Celsius)

    ThermistorReading(double adc, double resistance, double celsius)
      : _adc(adc), _resistance(resistance), _celsius(celsius) { }

    void print() {
      double f = _celsius * 1.8 + 32.0;
      Serial.print("adc = "); Serial.print(_adc); Serial.print(" r = "); Serial.print(_resistance); Serial.print(" C = "); Serial.print(_celsius); Serial.print(" F = "); Serial.println(f);
    }
};

class Thermistor {
  private:
    const double _k = 273.15;                 // 0 degrees Celsius in Kelvin

    // Constants used in Steinhart–Hart equation (below) are configureable via the
    // cloud and initialized during Thermistor::init().
    
    double _rs;                               // Value of resistor in voltage divider (in Ohms)
    double _r0;                               // Thermistor resistance at known temperature _t0 (in Ohms)
    double _t0;                               // Temperature at which thermistor has known resistance _r0 (in Kelvin)
    double _b;                                // 'B' coefficient in Steinhart-Hart equation

    // Convert ADC [0..1023] voltage reading to thermistor resistance
    double adcToResistance(double adc) {
      return _rs / ((1023.0 / adc) - 1.0);    // Solve for thermistor resistance in voltage divider.
    }

    // Calculates temperature in Celsius from thermistor resistance.
    double resistanceToCelsius(double r) {
      double steinhart;
      
      steinhart = r / _r0;                    // (R/R0)
      steinhart = log(steinhart);             // ln(R/R0)
      steinhart /= _b;                        // 1/B * ln(R/R0)
      steinhart += 1.0 / _t0;                 // + (1/T0)
      steinhart = 1.0 / steinhart;            // Invert
      steinhart -= _k;                        // Kelvin -> Celsius
       
      return steinhart;
    }

  public:
    void init(double rs, double r0, double t0, double b) {
      // Save constants used in Steinhart–Hart equation.
      _rs = rs;
      _r0 = r0;
      _t0 = t0 + _k;     // '+ _k' to convert from Celsius to Kelvin.
      _b  = b;
    }

    // Map raw ADC reading to thermistor resistance (in Ohms) and temperature (in Celsius).
    ThermistorReading toReading(double adc) {
      double resistance = adcToResistance(adc);
      double celsius = resistanceToCelsius(resistance);
      return ThermistorReading(adc, resistance, celsius);
    }
};

#endif // __THERMISTOR_H__
