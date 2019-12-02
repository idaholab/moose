
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
        KelvinUnit(),
        CelsiusUnit(),
        FahrenheitUnit()
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
        MillimeterUnit(),
        CentimeterUnit(),
        MeterUnit(),
        KilometerUnit(),
        InchUnit(),
        FeetUnit()
    ])

    def name(self):
        return "Length"


# Pressure units

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
        PascalUnit(),
        KilopascalUnit(),
        MegapascalUnit(),
        BarUnit()
    ])

    def name(self):
        return "Pressure"


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
        CubicMeterUnit(),
        USGallonUnit(),
        LiterUnit()
    ])

    def name(self):
        return "Volume"


# Unit groups

GROUPS = [
    TemperatureGroup(),
    LengthGroup(),
    PressureGroup(),
    SpeedGroup(),
    VolumeGroup()
]
