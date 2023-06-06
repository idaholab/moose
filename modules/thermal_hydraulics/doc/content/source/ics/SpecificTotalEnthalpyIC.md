# SpecificTotalEnthalpyIC

!syntax description /ICs/SpecificTotalEnthalpyIC

The cross-sectional area variable, [!param](/ICs/SpecificTotalEnthalpyIC/A),
is usually set by the [Component](syntax/Components/index.md).
For single-phase single-fluid thermal-hydraulics, the volume fraction,
[!param](/ICs/SpecificTotalEnthalpyIC/alpha), should remain
equal to one.

!alert note
This initial condition is usually added to the `Simulation` by the `FlowModel`, based on the parameters
passed to each [Component](syntax/Components/index.md).

!syntax parameters /ICs/SpecificTotalEnthalpyIC

!syntax inputs /ICs/SpecificTotalEnthalpyIC

!syntax children /ICs/SpecificTotalEnthalpyIC
