# TriInterWrapper1PhaseProblem

!syntax description /SubChannel/TriInterWrapper1PhaseProblem

##

<!-- -->

The inter-wrapper is the flow area around subchannel subassemblies.
This kernel solves for the flow variables in that area, for the case of subassembies that have subchannels in a triangular lattice.

The inter-wrapper solver is very similar to the subchannel solver. Information regarding the solver can be found in [subchannel_theory.md].

## Example Input File Syntax

!listing /test/tests/problems/interwrapper/tri_interwrapper_monolithic.i block=SubChannel language=cpp

!syntax parameters /SubChannel/TriInterWrapper1PhaseProblem

!syntax inputs /SubChannel/TriInterWrapper1PhaseProblem

!syntax children /SubChannel/TriInterWrapper1PhaseProblem
