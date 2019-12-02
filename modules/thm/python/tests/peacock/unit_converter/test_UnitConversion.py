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


if __name__ == '__main__':
    unittest.main(module = __name__, verbosity = 2)
