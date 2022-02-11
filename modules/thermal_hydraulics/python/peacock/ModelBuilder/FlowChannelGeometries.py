#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Flow channel geometries
"""

import math

class FlowChannelGeometry(object):
    def __init__(self):
        pass

    def name(self):
        """
        Return the name of the flow channel geometry
        """
        return self.NAME

    def compute(self, **kwargs):
        """
        Computes the flow channel parameters
        """
        raise NotImplementedError()

    def inputs(self, **kwargs):
        """
        Returns the input parameters needed to compute the flow channel parameters
        """
        raise NotImplementedError()

    def outputs(self):
        """
        Returns the parameters the flow channel
        """
        raise NotImplementedError()


class CircularRFlowChannelGeometry(FlowChannelGeometry):
    """
    Circular flow channel defined by radius
    """

    NAME = "Circular (radius)"

    def inputs(self):
        return [
            { 'name': "r", 'unit': "m", 'hint': "Radius" }
        ]

    def outputs(self):
        return [
            { 'name': "A", 'unit': "m^2", 'hint': "Cross-sectional area" },
            { 'name': "D_h", 'unit': "m", 'hint': "Hydraulic diameter" },
            { 'name': "P_hf", 'unit': "m", 'hint': "Heated perimeter" }
        ]

    def compute(self, **kwargs):
        r = kwargs['r']
        return {
            'A' : math.pi * r * r,
            'D_h' : 2 * r,
            'P_hf' : 2 * math.pi * r
        }


class CircularDFlowChannelGeometry(FlowChannelGeometry):
    """
    Circular flow channel defined by diameter
    """

    NAME = "Circular (diameter)"

    def inputs(self):
        return [
            { 'name': "d", 'unit': "m", 'hint': "Diameter" }
        ]

    def outputs(self):
        return [
            { 'name': "A", 'unit': "m^2", 'hint': "Cross-sectional area" },
            { 'name': "D_h", 'unit': "m", 'hint': "Hydraulic diameter" },
            { 'name': "P_hf", 'unit': "m", 'hint': "Heated perimeter" }
        ]

    def compute(self, **kwargs):
        d = kwargs['d']
        return {
            'A' : math.pi * d * d * 0.25,
            'D_h' : d,
            'P_hf' : math.pi * d
        }


class SquareFlowChannelGeometry(FlowChannelGeometry):
    """
    Square flow channel defined by the size of a side
    """

    NAME = "Square"

    def inputs(self):
        return [
            { 'name': "a", 'unit': "m", 'hint': "Side" }
        ]

    def outputs(self):
        return [
            { 'name': "A", 'unit': "m^2", 'hint': "Cross-sectional area" },
            { 'name': "D_h", 'unit': "m", 'hint': "Hydraulic diameter" },
            { 'name': "P_hf", 'unit': "m", 'hint': "Heated perimeter" }
        ]

    def compute(self, **kwargs):
        a = kwargs['a']
        return {
            'A' : a * a,
            'D_h' : a,
            'P_hf' : 4 * a
        }


class RectangularFlowChannelGeometry(FlowChannelGeometry):
    """
    Rectangular flow channel defined by two sides
    """

    NAME = "Rectangular"

    def inputs(self):
        return [
            { 'name': "a", 'unit': "m", 'hint': "Side one" },
            { 'name': "b", 'unit': "m", 'hint': "Side two" }
        ]

    def outputs(self):
        return [
            { 'name': "A", 'unit': "m^2", 'hint': "Cross-sectional area" },
            { 'name': "D_h", 'unit': "m", 'hint': "Hydraulic diameter" },
            { 'name': "P_hf", 'unit': "m", 'hint': "Heated perimeter" }
        ]

    def compute(self, **kwargs):
        a = kwargs['a']
        b = kwargs['b']
        return {
            'A' : a * b,
            'D_h' : 2 * a * b / (a + b),
            'P_hf' : 2 * (a + b)
        }


class AnnulusFlowChannelGeometry(FlowChannelGeometry):
    """
    Annulus flow channel defined by the inner and outer radius
    """

    NAME = "Annulus"

    def inputs(self):
        return [
            { 'name': "r_in", 'unit': "m", 'hint': "Inner radius" },
            { 'name': "r_out", 'unit': "m", 'hint': "Outer radius" }
        ]

    def outputs(self):
        return [
            { 'name': "A", 'unit': "m^2", 'hint': "Cross-sectional area" },
            { 'name': "D_h", 'unit': "m", 'hint': "Hydraulic diameter" },
            { 'name': "P_hf_in", 'unit': "m", 'hint': "Inner heated perimeter" },
            { 'name': "P_hf_out", 'unit': "m", 'hint': "Outer heated perimeter" },
        ]

    def compute(self, **kwargs):
        r_in = kwargs['r_in']
        r_out = kwargs['r_out']
        if r_in < r_out:
            D_in = 2 * r_in
            D_out = 2 * r_out
            return {
                'A' : math.pi * r_out * r_out - math.pi * r_in * r_in,
                'D_h' : D_out - D_in,
                'P_hf_in' : 2 * math.pi * r_in,
                'P_hf_out' : 2 * math.pi * r_out,
            }
        else:
            return {
                'A' : 0,
                'D_h' : 0,
                'P_hf_in' : 0,
                'P_hf_out' : 0,
                'error' : 'r_in has to be smaller than r_out'
            }


class CoreChannelRoundFuelFlowChannelGeometry(FlowChannelGeometry):
    """
    Rectangular flow channel around cylindrical fuel
    """

    NAME = "Core Channel (cylindrical fuel)"

    def inputs(self):
        return [
            { 'name': "pitch", 'unit': "m", 'hint': "Fuel pitch" },
            { 'name': "r", 'unit': "m", 'hint': "Fuel radius" }
        ]

    def outputs(self):
        return [
            { 'name': "A", 'unit': "m^2", 'hint': "Cross-sectional area" },
            { 'name': "D_h", 'unit': "m", 'hint': "Hydraulic diameter" },
            { 'name': "P_hf", 'unit': "m", 'hint': "Inner heated perimeter" }
        ]

    def compute(self, **kwargs):
        r = kwargs['r']
        pitch = kwargs['pitch']
        if 2 * r < pitch:
            A = pitch * pitch - math.pi * r * r
            P = 2. * math.pi * r
            return {
                'A' : A,
                'D_h' : 4. * A / P,
                'P_hf' : P,
            }
        else:
            return {
                'A' : 0,
                'D_h' : 0,
                'P_hf' : 0,
                'error' : 'pitch has to be larger than diameter'
            }


GEOMETRIES = (
    CircularRFlowChannelGeometry(),
    CircularDFlowChannelGeometry(),
    SquareFlowChannelGeometry(),
    AnnulusFlowChannelGeometry(),
    CoreChannelRoundFuelFlowChannelGeometry(),
    RectangularFlowChannelGeometry()
)
