# TriInterWrapper1PhaseProblem

!syntax description /Problem/TriInterWrapper1PhaseProblem

## Overview

!! Intentional comment to provide extra spacing

The inter-wrapper is the flow area around subchannel subassemblies.
This kernel solves for the flow variables in that area, for the case of sub-assembies that have subchannels/pins in a triangular lattice.

The inter-wrapper solver is very similar to the subchannel solver. Information regarding the subchannel solver can be found in [subchannel_theory.md].

Duct surface temperature is calculated at the end of the solve, if there is a PinMesh using Dittus Boelter correlation.

## Example Input File Syntax

!listing /test/tests/problems/interwrapper/tri_interwrapper_monolithic.i block=SubChannel language=moose

!syntax parameters /Problem/TriInterWrapper1PhaseProblem

!syntax inputs /Problem/TriInterWrapper1PhaseProblem

!syntax children /Problem/TriInterWrapper1PhaseProblem
