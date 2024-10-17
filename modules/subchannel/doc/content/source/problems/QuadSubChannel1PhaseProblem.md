# QuadSubChannel1PhaseProblem

!syntax description /Problem/QuadSubChannel1PhaseProblem

## Overview

<!-- -->

This class solves for the subchannel flow variables in the case of subchannels/pins arranged in a square lattice.
It inherits from the base class : `SubChannel1PhaseProblem`. Information regarding the solver can be found in [subchannel_theory.md].

## Example Input File Syntax

!listing /test/tests/problems/psbt/psbt_implicit.i block=SubChannel language=cpp

!syntax parameters /Problem/QuadSubChannel1PhaseProblem

!syntax inputs /Problem/QuadSubChannel1PhaseProblem

!syntax children /Problem/QuadSubChannel1PhaseProblem
