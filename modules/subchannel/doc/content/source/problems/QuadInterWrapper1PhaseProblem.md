# QuadInterWrapper1PhaseProblem

!syntax description /Problem/QuadInterWrapper1PhaseProblem

## Overview

<!-- -->

The inter-wrapper is the flow area around subchannel subassemblies.
This kernel solves for the flow variables in that area, for the case of sub-assembies that have subchannels/pins in a square lattice.

The inter-wrapper solver is very similar to the subchannel solver. Information regarding the subchannel solver can be found in [subchannel_theory.md].

## Example Input File Syntax

!listing /test/tests/problems/interwrapper/quad_interwrapper_monolithic.i block=SubChannel language=cpp

!syntax parameters /Problem/QuadInterWrapper1PhaseProblem

!syntax inputs /Problem/QuadInterWrapper1PhaseProblem

!syntax children /Problem/QuadInterWrapper1PhaseProblem
