# RadialDisplacementSphereAux

!syntax description /AuxKernels/RadialDisplacementSphereAux

Computing the radial displacement for spherically symmetric models is simply a matter
of reporting $u_r$.

For a 2D and 3D spherical models, the vector from the origin to a node is $p_{n0} = p_n - p_0$.  The radial displacement is then $u_r = u_n \cdot \frac{p_{n0}}{\left\lVert{p_{n0}}\right\rVert}$.

!syntax parameters /AuxKernels/RadialDisplacementSphereAux

!syntax inputs /AuxKernels/RadialDisplacementSphereAux

!syntax children /AuxKernels/RadialDisplacementSphereAux

!bibtex bibliography
