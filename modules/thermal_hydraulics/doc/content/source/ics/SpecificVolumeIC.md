# SpecificVolumeIC

!syntax description /ICs/SpecificVolumeIC

The cross-sectional area variable, [!param](/ICs/SpecificVolumeIC/A), is usually set by the [Component](syntax/Components/index.md).
For single-phase single-fluid thermal-hydraulics, the volume fraction, [!param](/ICs/SpecificVolumeIC/alpha), should remain
equal to one.

!alert note
This initial condition is usually added to the `Simulation` by the `FlowModel`, depending on the parameters
passed to each [Component](syntax/Components/index.md).

!syntax parameters /ICs/SpecificVolumeIC

!syntax inputs /ICs/SpecificVolumeIC

!syntax children /ICs/SpecificVolumeIC
