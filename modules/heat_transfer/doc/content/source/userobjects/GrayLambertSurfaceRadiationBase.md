# GrayLambertSurfaceRadiationBase

This is a base class for user objects that provide interfaces for getting radiation quantities
for [radiative exchange between opaque, gray, diffuse surfaces](modules/heat_transfer/index.md#gray_diffuse_radiative_exchange),
which are averaged over the requested surface:

| Interface | Description | SI Units | Symbol |
| :- | :- | :- | :- |
| getSurfaceIrradiation(id) | Irradiation | W/m$^2$ | $H_i$ |
| getSurfaceHeatFluxDensity(id) | Heat flux | W/m$^2$ | $q_i$ |
| getSurfaceTemperature(id) | Temperature | K | $T_i$ |
| getSurfaceRadiosity(id) | Radiosity | W/m$^2$ | $J_i$ |
| getSurfaceEmissivity(id) | Emissivity | - | $\epsilon_i$ |
| getViewFactor(from_id, to_id) | - | $F_{i,j}$ |
