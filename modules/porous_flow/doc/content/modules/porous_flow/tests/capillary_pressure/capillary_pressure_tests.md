# Capillary pressure test descriptions

The details of the capillary pressure curve implementations can be found in [here](porous_flow/capillary_pressure.md).

## Brooks-Corey

!media capillary_pressure/brook_corey.png style=width:100%;margin-left:10px; caption=Brooks-Corey Capillary Pressure test cases. id=brookcoreyPc

There are two test cases for Brooks-Corey including the log-extension turning on, and log-extension turning off at low saturation.

The input file for log-extension off test:

!listing modules/porous_flow/test/tests/capillary_pressure/brooks_corey1.i

The input file for log-extension on test:

!listing modules/porous_flow/test/tests/capillary_pressure/brooks_corey2.i



## van Genuchten



!media capillary_pressure/vangen.png style=width:100%;margin-left:10px; caption=van Genuchten Capillary Pressure test cases. id=vangenPc

There are three test cases for van Genuchten including the log-extension turning on, log-extension turning off at low saturation, and changing the scale factor.

The input file for log-extension off test:

!listing modules/porous_flow/test/tests/capillary_pressure/vangenuchten1.i

The input file for log-extension on test:

!listing modules/porous_flow/test/tests/capillary_pressure/vangenuchten2.i

The input file for applied-scaling test:

!listing modules/porous_flow/test/tests/capillary_pressure/vangenuchten3.i
