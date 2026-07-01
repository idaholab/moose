# GrayLambertSurfaceRadiationBase

This is a base class for user objects that provide interfaces for getting radiation quantities
for [gray, opaque, diffuse radiative heat transfer](modules/heat_transfer/index.md#gray_diffuse_radiative_exchange),
which are averaged over the requested surface:

| Interface | Description | Symbol |
| :- | :- | :- |
| getSurfaceIrradiation(id) | Irradiation | $H_i$ |
| getSurfaceHeatFluxDensity(id) | Heat flux | $q_i$ |
| getSurfaceTemperature(id) | Temperature | $T_i$ |
| getSurfaceRadiosity(id) | Radiosity | $J_i$ |
| getSurfaceEmissivity(id) | Emissivity | $\epsilon_i$ |
| getViewFactor(from_id, to_id) | $F_{i,j}$ |
