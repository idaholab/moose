# SubChannel User Notes

The following page is dedicated to various notes concerning SubChannel functionality
that might be helpful to both new and seasoned users. This page will be updated
periodically, so check back often!

## SubChannel index notation

SubChannel follows the following index notation for the subchannels, where black is the subchannel
index, white is the fuel-pin index and red is the gap index.

## Square lattice

!media figures/square_index.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=sindex
    caption=Square Lattice subchannel index notation, in a $5 \times 5$ sub-assembly.

## Triangular lattice

!media figures/hex_index.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=hindex
    caption=Triangular Lattice subchannel index notation, in a $3$ ring sub-assemly.

## Pressure boundary condition caveat

Subchannel's solver is designed to solve for relative pressure: $P_{relative} = P_{absolute} - P_{boundary}$, such that the relative pressure at the outlet boundary is zero. The pressure boundary condition is only explicitly used in the calculation of fluid properties. The user should be careful to initialize/set the pressure at the assembly outlet to zero (not setting it at all will also define the default value of zero).

## Related articles/papers

Development of a Single-Phase, Transient, Subchannel Code, within the MOOSE Multi-Physics Computational Framework [!cite](kyriakopoulos2022development)

Demonstration of Pronghorn’s Subchannel Code Modeling of Liquid-Metal Reactors and Validation in Normal Operation Conditions and Blockage Scenarios [!cite](kyriakopoulos2023demonstration)
