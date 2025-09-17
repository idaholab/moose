# TriSubChannel1PhaseProblem

!syntax description /Problem/PinTempSolver

## Overview

!! Intentional comment to provide extra spacing

This class solves for the steady state and transient U-Pu-Zr metal fuel pin temperature distribution coupled to subchannel flow solver.
It inherits from the base class : `TriSubChannel1PhaseProblem`. Information regarding the solver can be found in [subchannel_theory.md].


## Example Input File Syntax

!listing /test/tests/problems/SFR/pin_temp_solverSS/pin_tempss_.i block=Problem language=moose

!syntax parameters /Problem/PinTempSolver

!syntax inputs /Problem/PinTempSolver

!syntax children /Problem/PinTempSolver
