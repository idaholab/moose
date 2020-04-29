# INSADObjectTracker

The `INSADObjectTracker` user object is used to track the kernels added to an
incompressible Navier Stokes (INS) simulation. The object is then queried by
`INSADMaterial` to determine what material properties/strong residuals it needs
to calculate both for the base kernels and potentially for contributions from
PSPG and SUPG stabilization kernels. Creation of this object prevents
duplication of parameters between the kernels themselves and
`INSADMaterial`. For example, in the input file the user used to have to specify
to `INSADMaterial` whether there was a transient term in the
simulation. However, with the addition of `INSADObjectTracker` this input is no
longer required. By simply putting an `INSADMomentumTimeDerivative` object in
the input file, `INSADMaterial` will know to calculate a transient term because
of the `INSADObjectTracker`.

!syntax parameters /UserObjects/INSADObjectTracker

!syntax inputs /UserObjects/INSADObjectTracker

!syntax children /UserObjects/INSADObjectTracker
