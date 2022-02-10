#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

class Unit(object):

    def __init__(self):
        pass

    def name(self):
        """
        Human readable name of the unit

        @return[string] Unit name
        """
        return None

    def unit(self):
        return None

    def to(self, value):
        """
        Convert the 'value' from the common base unit into my unit

        @param value[float] Input value
        @return Converted value
        """
        return None

    def frm(self, value):
        """
        Convert the 'value' from my unit to the common base unit

        @param value[float] Input value
        @return Converted value
        """
        return None


class UnitGroup(object):
    """
    Groups units of the same type together
    """

    def __init__(self, units):
        """
        @param units[list]
        """
        self.units = units

    def name(self):
        """
        @return Human readable name of the unit group
        """
        return None


# Temperature units

class FahrenheitUnit(Unit):

    def name(self):
        return "Fahrenheit"

    def unit(self):
        return [ "F" ]

    def to(self, value):
        return (value - 32.) * 5. / 9. + 273.15

    def frm(self, value):
        return (value - 273.15) * 9. / 5. + 32.


class CelsiusUnit(Unit):

    def name(self):
        return "Celsius"

    def unit(self):
        return [ "C" ]

    def to(self, value):
        return value + 273.15

    def frm(self, value):
        return value - 273.15


class KelvinUnit(Unit):

    def name(self):
        return "Kelvin"

    def unit(self):
        return [ "K" ]

    def to(self, value):
        return value

    def frm(self, value):
        return value


class TemperatureGroup(UnitGroup):
    """
    Group of temperature units
    """
    def __init__(self):
        super(TemperatureGroup, self).__init__([
        FahrenheitUnit(),
        KelvinUnit(),
        CelsiusUnit()
    ])

    def name(self):
        return "Temperature"


# Length units

class MeterUnit(Unit):

    def name(self):
        return "Meter"

    def unit(self):
        return [ "m", "meter", "meters" ]

    def to(self, value):
        return value

    def frm(self, value):
        return value


class CentimeterUnit(Unit):

    def name(self):
        return "Centimeter"

    def unit(self):
        return [ "cm" ]

    def to(self, value):
        return value / 100.

    def frm(self, value):
        return value * 100


class MillimeterUnit(Unit):

    def name(self):
        return "Millimeter"

    def unit(self):
        return [ "mm" ]

    def to(self, value):
        return value / 1000.

    def frm(self, value):
        return value * 1000


class KilometerUnit(Unit):

    def name(self):
        return "Kilometer"

    def unit(self):
        return [ "km" ]

    def to(self, value):
        return value * 1000.

    def frm(self, value):
        return value / 1000


class InchUnit(Unit):

    def name(self):
        return "Inch"

    def unit(self):
        return [ "inch", "inches", "in", '"' ]

    def to(self, value):
        return value * 0.0254

    def frm(self, value):
        return value / 0.0254


class FeetUnit(Unit):

    def name(self):
        return "Feet"

    def unit(self):
        return [ "ft", "'", "feet", "foot" ]

    def to(self, value):
        return value * 0.3048

    def frm(self, value):
        return value / 0.3048


class LengthGroup(UnitGroup):
    """
    Group of length units
    """
    def __init__(self):
        super(LengthGroup, self).__init__([
        InchUnit(),
        FeetUnit(),
        KilometerUnit(),
        MeterUnit(),
        CentimeterUnit(),
        MillimeterUnit()
    ])

    def name(self):
        return "Length"


# Pressure units

class PsiUnit(Unit):

    def name(self):
        return "Pound-force per square inch"

    def unit(self):
        return [ "Psi", "psi", "psia" ]

    def to(self, value):
        return value * 6894.757

    def frm(self, value):
        return value / 6894.757

class PascalUnit(Unit):

    def name(self):
        return "Pascal"

    def unit(self):
        return [ "Pa" ]

    def to(self, value):
        return value

    def frm(self, value):
        return value


class KilopascalUnit(Unit):

    def name(self):
        return "Kilopascal"

    def unit(self):
        return [ "kPa" ]

    def to(self, value):
        return value * 1000

    def frm(self, value):
        return value / 1000


class MegapascalUnit(Unit):

    def name(self):
        return "Megapascal"

    def unit(self):
        return [ "MPa" ]

    def to(self, value):
        return value * 1000000

    def frm(self, value):
        return value / 1000000


class BarUnit(Unit):

    def name(self):
        return "Bar"

    def unit(self):
        return [ "bar" ]

    def to(self, value):
        return value * 100000

    def frm(self, value):
        return value / 100000


class PressureGroup(UnitGroup):
    """
    Group of pressure units
    """
    def __init__(self):
        super(PressureGroup, self).__init__([
        PsiUnit(),
        BarUnit(),
        MegapascalUnit(),
        KilopascalUnit(),
        PascalUnit()
    ])

    def name(self):
        return "Pressure"



# Mass flow rate units

class KilogramPerSecondUnit(Unit):

    def name(self):
        return "Kilogram per second"

    def unit(self):
        return [ "kg/s", "kg/sec" ]

    def to(self, value):
        return value

    def frm(self, value):
        return value


class PoundPerSecondUnit(Unit):

    def name(self):
        return "Pound per second"

    def unit(self):
        return [ "lb/s", "lbm/s", "lb/sec", "lbm/sec" ]

    def to(self, value):
        return value * 0.45359237

    def frm(self, value):
        return value / 0.45359237


class MegapoundPerHourlUnit(Unit):

    def name(self):
        return "Megapound per hour"

    def unit(self):
        return [ "Mlb/hr", "Mlbm/hr", "Mlbm/h", "Mlb/h" ]

    def to(self, value):
        return value * 1000000 * 0.45359237 / 3600

    def frm(self, value):
        return value * 3600 / (1000000 * 0.45359237)


class PoundPerMinutelUnit(Unit):

    def name(self):
        return "Pound per minute"

    def unit(self):
        return [ "lb/min", "lbm/min" ]

    def to(self, value):
        return value * 0.45359237 / 60

    def frm(self, value):
        return value * 60 /  0.45359237


class PoundPerHourUnit(Unit):

    def name(self):
        return "Pound per hour"

    def unit(self):
        return [ "lb/hr", "lbm/hr", "lbm/h", "lb/h" ]

    def to(self, value):
        return value * 0.45359237 / 3600

    def frm(self, value):
        return value * 3600 / 0.45359237


class MassFlowRateGroup(UnitGroup):
    """
    Group of mass flow rate units
    """
    def __init__(self):
        super(MassFlowRateGroup, self).__init__([
        PoundPerSecondUnit(),
        PoundPerMinutelUnit(),
        PoundPerHourUnit(),
        MegapoundPerHourlUnit(),
        KilogramPerSecondUnit()
    ])

    def name(self):
        return "Mass flow rate"


# Speed

class FootPerSecondUnit(Unit):

    def name(self):
        return "Foot per second"

    def unit(self):
        return [ "ft/s" ]

    def to(self, value):
        return value * 0.3048

    def frm(self, value):
        return value / 0.3048


class MeterPerSecondUnit(Unit):

    def name(self):
        return "Meter per second"

    def unit(self):
        return [ "m/s" ]

    def to(self, value):
        return value

    def frm(self, value):
        return value


class SpeedGroup(UnitGroup):
    """
    Group of speed units
    """
    def __init__(self):
        super(SpeedGroup, self).__init__([
        FootPerSecondUnit(),
        MeterPerSecondUnit()
    ])

    def name(self):
        return "Speed"


# Volume group

class CubicMeterUnit(Unit):

    def name(self):
        return "Cubic meter"

    def unit(self):
        return [ "m^3" ]

    def to(self, value):
        return value

    def frm(self, value):
        return value


class USGallonUnit(Unit):

    def name(self):
        return "US Gallon"

    def unit(self):
        return [ "gal" ]

    def to(self, value):
        return value * 0.00378541

    def frm(self, value):
        return value / 0.00378541


class LiterUnit(Unit):

    def name(self):
        return "Liter"

    def unit(self):
        return [ "l" ]

    def to(self, value):
        return value / 1000

    def frm(self, value):
        return value * 1000


class VolumeGroup(UnitGroup):
    """
    Group of volume units
    """
    def __init__(self):
        super(VolumeGroup, self).__init__([
        USGallonUnit(),
        CubicMeterUnit(),
        LiterUnit()
    ])

    def name(self):
        return "Volume"


# Mass group

class KilogramUnit(Unit):

    def name(self):
        return "Kilogram"

    def unit(self):
        return [ "kg" ]

    def to(self, value):
        return value

    def frm(self, value):
        return value


class GramUnit(Unit):

    def name(self):
        return "Gram"

    def unit(self):
        return [ "g" ]

    def to(self, value):
        return value / 1000

    def frm(self, value):
        return value * 1000


class PoundUnit(Unit):

    def name(self):
        return "Pound"

    def unit(self):
        return [ "lbs" ]

    def to(self, value):
        return value * 0.453592

    def frm(self, value):
        return value / 0.453592


class MassGroup(UnitGroup):
    """
    Group of mass units
    """
    def __init__(self):
        super(MassGroup, self).__init__([
        PoundUnit(),
        KilogramUnit(),
        GramUnit(),
    ])

    def name(self):
        return "Mass"


# Energy/enthalpy units

class JoulePerKilogramUnit(Unit):

    def name(self):
        return "Joule per kilogram"

    def unit(self):
        return [ "J/kg", "Joule/kg" ]

    def to(self, value):
        return value

    def frm(self, value):
        return value


class KilojoulePerKilogramUnit(Unit):

    def name(self):
        return "Kilojoule per kilogram"

    def unit(self):
        return [ "kJ/kg" ]

    def to(self, value):
        return value * 1000

    def frm(self, value):
        return value / 1000


class BtuPerPoundUnit(Unit):

    def name(self):
        return "British thermal unit per pound"

    def unit(self):
        return [ "BTU/lb", "Btu/lb", "BTU/lbm", "Btu/lbm" ]

    def to(self, value):
        return value * 1055.06 / 0.45359237

    def frm(self, value):
        return value * 0.45359237 / 1055.06


class EnergyGroup(UnitGroup):
    """
    Group of energy units
    """
    def __init__(self):
        super(EnergyGroup, self).__init__([
        BtuPerPoundUnit(),
        JoulePerKilogramUnit(),
        KilojoulePerKilogramUnit()
    ])

    def name(self):
        return "Energy"


# Time units

class SecondUnit(Unit):

    def name(self):
        return "Second"

    def unit(self):
        return [ "s", "sec", "secs", "second", "seconds" ]

    def to(self, value):
        return value

    def frm(self, value):
        return value


class MinuteUnit(Unit):

    def name(self):
        return "Minute"

    def unit(self):
        return [ "min", "mins", "minute", "minutes" ]

    def to(self, value):
        return value * 60.

    def frm(self, value):
        return value / 60.


class HourUnit(Unit):

    def name(self):
        return "Hour"

    def unit(self):
        return [ "h", "hr", "hrs", "hour", "hours" ]

    def to(self, value):
        return value * 3600.

    def frm(self, value):
        return value / 3600.


class DayUnit(Unit):

    def name(self):
        return "Day"

    def unit(self):
        return [ "day", "days" ]

    def to(self, value):
        return value * 86400.

    def frm(self, value):
        return value / 86400.


class YearUnit(Unit):

    def name(self):
        return "Year"

    def unit(self):
        return [ "year", "years" ]

    def to(self, value):
        # assuming 365 days in a year
        return value * 31536000.

    def frm(self, value):
        return value / 31536000.


class TimeGroup(UnitGroup):
    """
    Group of time units
    """
    def __init__(self):
        super(TimeGroup, self).__init__([
        SecondUnit(),
        MinuteUnit(),
        HourUnit(),
        DayUnit(),
        YearUnit()
    ])

    def name(self):
        return "Time"

# Unit groups

GROUPS = [
    TemperatureGroup(),
    LengthGroup(),
    PressureGroup(),
    SpeedGroup(),
    VolumeGroup(),
    MassGroup(),
    MassFlowRateGroup(),
    EnergyGroup(),
    TimeGroup()
]
