# SCM User Notes

The following page is presenting various notes concerning SCM functionality
that might be helpful to both new and seasoned users. This page will be updated
periodically, so check back often!

## SCM index notation

SCM follows the index notation presented in the two figures bellow: where in black numbers is the subchannel
index, in white numbers the fuel-pin index and in red numbers the gap index. For both meshes (square, hexagonal) the center of the mesh is the origin.

### Square lattice

!media subchannel/getting_started/square_index.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=sindex
    caption=Square Lattice subchannel index notation, in a $5 \times 5$ sub-assembly.

### Triangular lattice

!media subchannel/getting_started/hex_index.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=hindex
    caption=Triangular Lattice subchannel index notation, in a $3$ ring sub-assemly.

In order for the user to print the index notation for subchannels and pins in a triangualar lattice arrangement, they are invited to run the following Python script:

!listing /scripts/index.py

## Pressure boundary condition caveat

SCM's solver is designed to solve for relative pressure: $P_{relative} = P_{absolute} - P_{boundary}$, such that the relative pressure at the outlet boundary is always zero. The pressure boundary condition is only explicitly used in the calculation of fluid properties. The user should be careful to initialize/set the pressure at the assembly outlet to zero (not setting it at all will also define the default value of zero).

## The displacement variable

The 'displacement' variable is used as a way to model a deformed duct. It's a per subchannel variable that affects the calculation of the geometric parameters (wetted perimeter, flow area) [!cite](kyriakopoulos2024validation) of the subchannels next to the duct.

## Naming/Branding

Initially SCM was part of [Pronghorn](https://mooseframework.inl.gov/ncrc/applications/ncrc_root_pronghorn.html). It was called `Pronghorn-Subchannel`, `Pr-Sub`. Once the code became a [MOOSE](https://mooseframework.inl.gov/index.html) module it has been re-branded as SCM.
