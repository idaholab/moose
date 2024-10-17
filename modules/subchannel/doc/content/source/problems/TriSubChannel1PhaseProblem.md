# TriSubChannel1PhaseProblem

!syntax description /Problem/TriSubChannel1PhaseProblem

## Overview

<!-- -->

This class solves for the subchannel flow variables in the case of subchannels/pins arranged in a triangular lattice.
It inherits from the base class : `SubChannel1PhaseProblem`. Information regarding the solver can be found in [subchannel_theory.md].

## Example Input File Syntax

!listing /test/tests/problems/Lead-LBE-19pin/test_LEAD-19pin.i block=Problem language=cpp

!syntax parameters /Problem/TriSubChannel1PhaseProblem

!syntax inputs /Problem/TriSubChannel1PhaseProblem

!syntax children /Problem/TriSubChannel1PhaseProblem
