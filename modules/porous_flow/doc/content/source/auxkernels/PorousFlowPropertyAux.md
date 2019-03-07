# PorousFlowPropertyAux

!syntax description /AuxKernels/PorousFlowPropertyAux

This `AuxKernel` provides simplified access to fluid and material properties. The
following properties are available using the `property` input parameter:

- `pressure`
- `saturation`
- `temperature`
- `density`
- `viscosity`
- `mass_fraction`
- `relperm`
- `capillary_pressure`
- `enthalpy`
- `internal_energy`
- `secondary_concentration` (m$^{3}$(secondary species)/m$^{3}$(fluid))
- `mineral_concentration` (m$^{3}$(secondary species)/m$^{3}$(porous material))
- `mineral_reaction_rate` (m$^{3}$(secondary species).m$^{-3}$(porous material).s$^{-1}$))
- `porosity`
- `permeability`

The fluid phase and fluid component are specified in the `phase` and
`fluid_component` input parameters, respectively.  For properties
related to chemical reactions, the `secondary_species` and
`mineral_species` parameters are relevant.

!!! note:
    As this `AuxKernel` uses material properties, only elemental (`Monomial`) `AuxVariables`
    can be used.

!syntax parameters /AuxKernels/PorousFlowPropertyAux

!syntax inputs /AuxKernels/PorousFlowPropertyAux

!syntax children /AuxKernels/PorousFlowPropertyAux
