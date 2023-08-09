# Heat Transfer Models

The `HeatTransferModels` namespace provides various model functions relevant to heat transfer.

## Cylindrical Thermal Conductance id=cylindrical_thermal_conductance

The function `cylindricalThermalConductance` computes the thermal conductance
across a cylindrical medium using a steady thermal resistance analysis [!citep](incropera2002):

!equation id=eq:cylindrical_thermal_conductance
\mathcal{H} = \frac{k}{\bar{r} \ln(r_o / r_i)} \,,

where

- $r_i$ is the inner surface radius,
- $r_o$ is the outer surface radius,
- $\bar{r}$ is the arithmetic mean radius, and
- $k$ is the medium thermal conductivity.

## Cylindrical Gap Conduction Heat Flux id=cylindrical_gap_conduction_heat_flux

The function `cylindricalGapConductionHeatFlux` computes the heat flux $q$ at a point
across a cylindrical gap due to conduction, using the thermal conductance given
in [#cylindrical_thermal_conductance]:

!equation
q = \mathcal{H} (T_i - T_o) \,,

where

- $T_i$ is the inner surface temperature,
- $T_o$ is the outer surface temperature, and
- $\mathcal{H}$ is computed from [!eqref](eq:cylindrical_thermal_conductance).

Note that the convention here is that a positive heat flux corresponds to heat
moving from the inner surface to the outer surface.

## Cylindrical Gap Radiation Heat Flux id=cylindrical_gap_radiation_heat_flux

The function `cylindricalGapRadiationHeatFlux` computes heat flux $q$ at a point
across a cylindrical gap due to radiation, assuming opaque, gray, diffuse surfaces
with infinitely long, concentric cylinders [!citep](incropera2002):

!equation
q = \frac{\sigma (T_i^4 - T_o^4)}{\mathcal{R}} \,,

!equation
\mathcal{R} = \frac{1}{\epsilon_i} + \frac{r_i}{r_o}
\left( \frac{1 - \epsilon_o}{\epsilon_o} \right) \,,

where $\sigma$ is the Stefan-Boltzmann constant.

Note that the convention here is that a positive heat flux corresponds to heat
moving from the inner surface to the outer surface.
