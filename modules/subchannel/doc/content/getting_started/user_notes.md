# SCM User Notes

The following page is dedicated to various notes concerning SCM functionality
that might be helpful to both new and seasoned users. This page will be updated
periodically, so check back often!

## SCM index notation

SCM follows the following index notation for the subchannels, where black is the subchannel
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

SCM's solver is designed to solve for relative pressure: $P_{relative} = P_{absolute} - P_{boundary}$, such that the relative pressure at the outlet boundary is zero. The pressure boundary condition is only explicitly used in the calculation of fluid properties. The user should be careful to initialize/set the pressure at the assembly outlet to zero (not setting it at all will also define the default value of zero).

## Naming/Branding

Initially SCM was part of [Pronghorn](https://mooseframework.inl.gov/ncrc/applications/ncrc_root_pronghorn.html). It was called `Pronghorn-Subchannel`, `Pr-Sub`. Once the code became a [MOOSE](https://mooseframework.inl.gov/index.html) module it has been re-branded as SCM.
