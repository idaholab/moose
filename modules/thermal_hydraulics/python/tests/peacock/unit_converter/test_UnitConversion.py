#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", 'peacock'))
import unittest
from UnitConversion import *

class TestUnitConversion(unittest.TestCase):
    """
    Tests for UnitConversion
    """

    def setUp(self):
        pass

    def testTemperature(self):
        fahrenheit = FahrenheitUnit()
        self.assertTrue(fahrenheit.name() == 'Fahrenheit')
        self.assertTrue(fahrenheit.unit() == [ 'F' ])
        self.assertAlmostEqual(fahrenheit.to(32.), 273.15, places = 6)
        self.assertAlmostEqual(fahrenheit.frm(273.15), 32., places = 6)

        kelvin = KelvinUnit()
        self.assertTrue(kelvin.name() == 'Kelvin')
        self.assertTrue(kelvin.unit() == [ 'K' ])
        self.assertAlmostEqual(kelvin.to(123.3456), 123.3456, places = 6)
        self.assertAlmostEqual(kelvin.frm(123.3456), 123.3456, places = 6)

        celsius = CelsiusUnit()
        self.assertTrue(celsius.name() == 'Celsius')
        self.assertTrue(celsius.unit() == [ 'C' ])
        self.assertAlmostEqual(celsius.to(0), 273.15, places = 6)
        self.assertAlmostEqual(celsius.frm(273.15), 0., places = 6)

        temp = TemperatureGroup()
        self.assertTrue(temp.name() == 'Temperature')


    def testLength(self):
        meter = MeterUnit()
        self.assertTrue(meter.name() == 'Meter')
        self.assertTrue(meter.unit() == [ "m", "meter", "meters" ])
        self.assertAlmostEqual(meter.to(123.456), 123.456, places = 6)
        self.assertAlmostEqual(meter.frm(123.456), 123.456, places = 6)

        centimeter = CentimeterUnit()
        self.assertTrue(centimeter.name() == 'Centimeter')
        self.assertTrue(centimeter.unit() == [ "cm" ])
        self.assertAlmostEqual(centimeter.to(123.456), 1.23456, places = 6)
        self.assertAlmostEqual(centimeter.frm(1.23456), 123.456, places = 6)

        millimeter = MillimeterUnit()
        self.assertTrue(millimeter.name() == 'Millimeter')
        self.assertTrue(millimeter.unit() == [ "mm" ])
        self.assertAlmostEqual(millimeter.to(123.456), 0.123456, places = 6)
        self.assertAlmostEqual(millimeter.frm(0.123456), 123.456, places = 6)

        kilometer = KilometerUnit()
        self.assertTrue(kilometer.name() == 'Kilometer')
        self.assertTrue(kilometer.unit() == [ "km" ])
        self.assertAlmostEqual(kilometer.to(1.23456), 1234.56, places = 6)
        self.assertAlmostEqual(kilometer.frm(1234.56), 1.23456, places = 6)

        inch = InchUnit()
        self.assertTrue(inch.name() == 'Inch')
        self.assertTrue(inch.unit() == [ "inch", "inches", "in", '"' ])
        self.assertAlmostEqual(inch.to(1.), 0.0254, places = 6)
        self.assertAlmostEqual(inch.frm(0.0254), 1., places = 6)

        feet = FeetUnit()
        self.assertTrue(feet.name() == 'Feet')
        self.assertTrue(feet.unit() == [ "ft", "'", "feet", "foot" ])
        self.assertAlmostEqual(feet.to(1.), 0.3048, places = 6)
        self.assertAlmostEqual(feet.frm(0.3048), 1., places = 6)

        length = LengthGroup()
        self.assertTrue(length.name() == 'Length')


    def testPressure(self):
        pascal = PascalUnit()
        self.assertTrue(pascal.name() == 'Pascal')
        self.assertTrue(pascal.unit() == [ "Pa" ])
        self.assertAlmostEqual(pascal.to(123.456), 123.456, places = 6)
        self.assertAlmostEqual(pascal.frm(123.456), 123.456, places = 6)

        kilopascal = KilopascalUnit()
        self.assertTrue(kilopascal.name() == 'Kilopascal')
        self.assertTrue(kilopascal.unit() == [ "kPa" ])
        self.assertAlmostEqual(kilopascal.to(123.456), 123456., places = 6)
        self.assertAlmostEqual(kilopascal.frm(123456.), 123.456, places = 6)

        megapascal = MegapascalUnit()
        self.assertTrue(megapascal.name() == 'Megapascal')
        self.assertTrue(megapascal.unit() == [ "MPa" ])
        self.assertAlmostEqual(megapascal.to(0.123456), 123456., places = 6)
        self.assertAlmostEqual(megapascal.frm(123456.), 0.123456, places = 6)

        bar = BarUnit()
        self.assertTrue(bar.name() == 'Bar')
        self.assertTrue(bar.unit() == [ "bar" ])
        self.assertAlmostEqual(bar.to(0.123456), 12345.6, places = 6)
        self.assertAlmostEqual(bar.frm(12345.6), 0.123456, places = 6)

        psi = PsiUnit()
        self.assertTrue(psi.name() == 'Pound-force per square inch')
        self.assertTrue(psi.unit() == [ "Psi", "psi", "psia" ])
        self.assertAlmostEqual(psi.to(1.), 6894.757, places = 6)
        self.assertAlmostEqual(psi.frm(6894.757), 1.0, places = 6)

        pressure = PressureGroup()
        self.assertTrue(pressure.name() == 'Pressure')


    def testSpeed(self):
        ftps = FootPerSecondUnit()
        self.assertTrue(ftps.name() == 'Foot per second')
        self.assertTrue(ftps.unit() == [ "ft/s" ])
        self.assertAlmostEqual(ftps.to(1.), 0.3048, places = 6)
        self.assertAlmostEqual(ftps.frm(0.3048), 1., places = 6)

        mps = MeterPerSecondUnit()
        self.assertTrue(mps.name() == 'Meter per second')
        self.assertTrue(mps.unit() == [ "m/s" ])
        self.assertAlmostEqual(mps.to(123.456), 123.456, places = 6)
        self.assertAlmostEqual(mps.frm(123.456), 123.456, places = 6)

        speed = SpeedGroup()
        self.assertTrue(speed.name() == 'Speed')


    def testVolume(self):
        cubm = CubicMeterUnit()
        self.assertTrue(cubm.name() == 'Cubic meter')
        self.assertTrue(cubm.unit() == [ "m^3" ])
        self.assertAlmostEqual(cubm.to(123.456), 123.456, places = 6)
        self.assertAlmostEqual(cubm.frm(123.456), 123.456, places = 6)

        gal = USGallonUnit()
        self.assertTrue(gal.name() == 'US Gallon')
        self.assertTrue(gal.unit() == [ "gal" ])
        self.assertAlmostEqual(gal.to(1.), 0.00378541, places = 6)
        self.assertAlmostEqual(gal.frm(0.00378541), 1., places = 6)

        liter = LiterUnit()
        self.assertTrue(liter.name() == 'Liter')
        self.assertTrue(liter.unit() == [ "l" ])
        self.assertAlmostEqual(liter.to(1000.), 1, places = 6)
        self.assertAlmostEqual(liter.frm(1), 1000., places = 6)

        volume = VolumeGroup()
        self.assertTrue(volume.name() == 'Volume')


    def testMass(self):
        kg = KilogramUnit()
        self.assertTrue(kg.name() == 'Kilogram')
        self.assertTrue(kg.unit() == [ "kg" ])
        self.assertAlmostEqual(kg.to(123.456), 123.456, places = 6)
        self.assertAlmostEqual(kg.frm(123.456), 123.456, places = 6)

        g = GramUnit()
        self.assertTrue(g.name() == 'Gram')
        self.assertTrue(g.unit() == [ "g" ])
        self.assertAlmostEqual(g.to(123.456), 0.123456, places = 6)
        self.assertAlmostEqual(g.frm(0.123456), 123.456, places = 6)

        lbs = PoundUnit()
        self.assertTrue(lbs.name() == 'Pound')
        self.assertTrue(lbs.unit() == [ "lbs" ])
        self.assertAlmostEqual(lbs.to(1.), 0.453592, places = 6)
        self.assertAlmostEqual(lbs.frm(0.453592), 1., places = 6)

        mass = MassGroup()
        self.assertTrue(mass.name() == 'Mass')


    def testMassFlowRate(self):
        kgpersecond = KilogramPerSecondUnit()
        self.assertTrue(kgpersecond.name() == 'Kilogram per second')
        self.assertTrue(kgpersecond.unit() == [ "kg/s", "kg/sec" ])
        self.assertAlmostEqual(kgpersecond.to(123.456), 123.456, places = 6)
        self.assertAlmostEqual(kgpersecond.frm(123.456), 123.456, places = 6)

        lbpersecond = PoundPerSecondUnit()
        self.assertTrue(lbpersecond.name() == 'Pound per second')
        self.assertTrue(lbpersecond.unit() == [ "lb/s", "lbm/s", "lb/sec", "lbm/sec" ])
        self.assertAlmostEqual(lbpersecond.to(1.), 0.45359237, places = 6)
        self.assertAlmostEqual(lbpersecond.frm(0.45359237), 1., places = 6)

        lbperhour = PoundPerHourUnit()
        self.assertTrue(lbperhour.name() == 'Pound per hour')
        self.assertTrue(lbperhour.unit() == [ "lb/hr", "lbm/hr", "lbm/h", "lb/h" ])
        self.assertAlmostEqual(lbperhour.to(1.), 1.259978805556e-4, places = 6)
        self.assertAlmostEqual(lbperhour.frm(1.259978805556e-4), 1., places = 6)

        megalbperhour = MegapoundPerHourlUnit()
        self.assertTrue(megalbperhour.name() == 'Megapound per hour')
        self.assertTrue(megalbperhour.unit() == [ "Mlb/hr", "Mlbm/hr", "Mlbm/h", "Mlb/h" ])
        self.assertAlmostEqual(megalbperhour.to(1.), 125.9978805556, places = 6)
        self.assertAlmostEqual(megalbperhour.frm(125.9978805556), 1., places = 6)

        lbperminute = PoundPerMinutelUnit()
        self.assertTrue(lbperminute.name() == 'Pound per minute')
        self.assertTrue(lbperminute.unit() == [ "lb/min", "lbm/min" ])
        self.assertAlmostEqual(lbperminute.to(1.), 0.007559872833, places = 6)
        self.assertAlmostEqual(lbperminute.frm(0.007559872833), 1., places = 6)

        mdot = MassFlowRateGroup()
        self.assertTrue(mdot.name() == 'Mass flow rate')


    def testEnergy(self):
        Jperkg = JoulePerKilogramUnit()
        self.assertTrue(Jperkg.name() == 'Joule per kilogram')
        self.assertTrue(Jperkg.unit() == [ "J/kg", "Joule/kg" ])
        self.assertAlmostEqual(Jperkg.to(123.456), 123.456, places = 6)
        self.assertAlmostEqual(Jperkg.frm(123.456), 123.456, places = 6)

        kJperkg = KilojoulePerKilogramUnit()
        self.assertTrue(kJperkg.name() == 'Kilojoule per kilogram')
        self.assertTrue(kJperkg.unit() == [ "kJ/kg" ])
        self.assertAlmostEqual(kJperkg.to(1.), 1000., places = 6)
        self.assertAlmostEqual(kJperkg.frm(1000), 1., places = 6)

        btuperlb = BtuPerPoundUnit()
        self.assertTrue(btuperlb.name() == 'British thermal unit per pound')
        self.assertTrue(btuperlb.unit() == [ "BTU/lb", "Btu/lb", "BTU/lbm", "Btu/lbm" ])
        self.assertAlmostEqual(btuperlb.to(1.), 2326.0091434078, places = 6)
        self.assertAlmostEqual(btuperlb.frm(2326.0091434078), 1., places = 6)

        energy = EnergyGroup()
        self.assertTrue(energy.name() == 'Energy')


    def testTime(self):
        sec = SecondUnit()
        self.assertTrue(sec.name() == 'Second')
        self.assertTrue(sec.unit() == [ "s", "sec", "secs", "second", "seconds" ])
        self.assertAlmostEqual(sec.to(123.456), 123.456, places = 6)
        self.assertAlmostEqual(sec.frm(123.456), 123.456, places = 6)

        min = MinuteUnit()
        self.assertTrue(min.name() == 'Minute')
        self.assertTrue(min.unit() == [ "min", "mins", "minute", "minutes" ])
        self.assertAlmostEqual(min.to(1.), 60., places = 6)
        self.assertAlmostEqual(min.frm(60), 1., places = 6)

        hour = HourUnit()
        self.assertTrue(hour.name() == 'Hour')
        self.assertTrue(hour.unit() == [ "h", "hr", "hrs", "hour", "hours" ])
        self.assertAlmostEqual(hour.to(1.), 3600, places = 6)
        self.assertAlmostEqual(hour.frm(3600.), 1., places = 6)

        day = DayUnit()
        self.assertTrue(day.name() == 'Day')
        self.assertTrue(day.unit() == [ "day", "days" ])
        self.assertAlmostEqual(day.to(1.), 86400., places = 6)
        self.assertAlmostEqual(day.frm(86400.), 1., places = 6)

        year = YearUnit()
        self.assertTrue(year.name() == 'Year')
        self.assertTrue(year.unit() == [ "year", "years" ])
        self.assertAlmostEqual(year.to(1.), 31536000., places = 6)
        self.assertAlmostEqual(year.frm(31536000.), 1., places = 6)

        t = TimeGroup()
        self.assertTrue(t.name() == 'Time')

if __name__ == '__main__':
    unittest.main(module = __name__, verbosity = 2)
