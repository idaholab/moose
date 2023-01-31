# Turbulence modeling in MOOSE

!---

## Adding the mixing model via the Navier Stokes Action

!listing fluid-only-turbulent.i
         block=NavierStokesFV

The mixing-length model can be directly added via the ```NSFVAction```

!---

## A few extra tests:

- Testing the laminar and turbulent models against correlations
- Understanding the influence of $\ell_m$ and $\kappa$ on the model behaviour


