# QuadInterWrapper1PhaseProblem

!syntax description /SubChannel/QuadInterWrapper1PhaseProblem

## Overview

<!-- -->

The inter-wrapper is the flow area around subchannel subassemblies.
This kernel solves for the flow variables in that area, for the case of subassembies that have subchannels in a square lattice.

The inter-wrapper solver is very similar to the subchannel solver. Information regarding the solver can be found in [subchannel_theory.md].

## Example Input File Syntax

!listing /test/tests/problems/interwrapper/quad_interwrapper_monolithic.i block=SubChannel language=cpp

!syntax parameters /SubChannel/QuadInterWrapper1PhaseProblem

!syntax inputs /SubChannel/QuadInterWrapper1PhaseProblem

!syntax children /SubChannel/QuadInterWrapper1PhaseProblem
