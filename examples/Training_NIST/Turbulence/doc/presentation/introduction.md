# Physics Coupling in MOOSE

!---

## Turbulence modeling in MOOSE

- Turbulence models are implemented in MOOSE (not Pronghorn)

- Currently the MOOSE-FV Navier Stokes module supports a simple capped mixing length turbulence model

- These models can already support many engineering applications

- However, to increase support, there are a variety of new two-equation turbulence models coming to MOOSE as a part of FY23 efforts. The main ones are:

  -  Standard $k-\epsilon$ turbulence model (merge request in progress)
  -  Low-Re number $k-\epsilon$ turbulence model
  -  Willcox $k-\omega$ SST turbulence model

!---

# Content

In the following session we will go through some of the details of the capped mixing length model available in MOOSE-FV.



