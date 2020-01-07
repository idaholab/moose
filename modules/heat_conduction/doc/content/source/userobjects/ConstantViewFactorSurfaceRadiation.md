# ConstantViewFactorSurfaceRadiation

## Description

`ConstantViewFactorSurfaceRadiation` implements the exchange of heat by radiation between
sidesets by the net radiation method.
The net radiation method is valid if the surfaces are gray, diffuse radiators.
The purpose of the `ConstantViewFactorSurfaceRadiation` is to provide the means
to model radiative exchange for coarse-grained model. `ConstantViewFactorSurfaceRadiation`
does not compute radiative exchange between element surfaces, but it computes radiative
transfer only in an average sense between sidesets.

The `ConstantViewFactorSurfaceRadiation` allows coupling radiative heat transfer to regions
where the heat equation is solved. The net heat transfer caused by radiative heat transfer is
coupled to the temperature field by `GrayLambertNeumannBC`. `ConstantViewFactorSurfaceRadiation` also supports the
definition of adiabatic and fixed temperature sidesets. The temperature variable, i.e. the
variable of the heat conduction equation, does not need to be defined on adiabatic and fixed
temperature boundaries. This is particularly useful in cavities, where temperature is only
defined on sidesets immediately adjacent to the heat conduction domain and not on the
adiabatic and isothermal walls enclosing the cavity. There are three different types of boundary
conditions in `ConstantViewFactorSurfaceRadiation`:

- `VARIABLE_TEMPERATURE` are sidesets where temperature is provided by the `temperature` variable.
  The heat equation is coupled to the `ConstantViewFactorSurfaceRadiation` through these boundaries.

- `FIXED_TEMPERATURE` are sidesets with temperature given as a function. The difference to `VARIABLE_TEMPERATURE`
  is that we do not need to solve for temperature on `FIXED_TEMPERATURE`. `FIXED_TEMPERATURE` sidesets
  can for example be the outside of a cavity that is in radiative heat transfer with the domain
  but is kept at constant temperature by being in contact with an effective coolant.

- `ADIABATIC` sidesets have a zero net heat-flux. This implies that the inflow of radiation has
  to equal the outflow of radiation. The surface temperature assumes the value at which this condition
  is true. `ADIABATIC` sidesets do not require the temperature variable to be defined.

## Explanation of the input parameters

This paragraph describes the input structure of the `ConstantViewFactorSurfaceRadiation` object.
The following parameters are defined in detail:

- `temperature`: the user must provide the name of the temperature variable as the `temperature` parameter.
  `temperature` is the variable that the heat conduction equation solves for.

- `boundary` contains the names of _all_ sidesets that participate in the radiative heat exchange.

- `fixed_temperature_boundary` should list all sidesets that are fixed temperature boundaries.
  `fixed_temperature_boundary` must be a subset of the `boundary` array.

- `fixed_boundary_temperatures` contains function names specifying the temperatures on the fixed
  temperature boundaries. `fixed_boundary_temperatures` must be the same length as `fixed_temperature_boundary`.

- `adiabatic_boundary` should list all sidesets that are adiabatic boundaries.
  `adiabatic_boundary` must be a subset of the `boundary` array.

- `view_factors` lists the view factors $F_{i,j}$ from sideset $i$ to sideset $j$. The ordering of the
  `view_factors` follows the ordering in `boundary`. View factors must be added as square arrays. This may
  appear redundant because missing view factors can be computed using reciprocity, but this makes it impossible
  to check that each row of the view factor matrix sums to 1. This is absolutely necessary for conserving
  energy and ensuring stability of a coupled radiative transfer, heat conduction calculation. Row sums that
  are less than 5 percent off, are corrected to sum to 1. If the row sum is off by more than this threshold,
  an error is thrown. This is an example of the format of `view_factors` for three surfaces:

  \begin{equation}
      '~~F_{1,1} F_{1,2} F_{1,3} ;
       F_{2,1} F_{2,2} F_{2,3} ;
       F_{3,1} F_{3,2} F_{3,3} ~~'
  \end{equation}

## Example Input syntax

!listing modules/heat_conduction/test/tests/gray_lambert_radiator/gray_lambert_cavity.i
block=UserObjects

!syntax parameters /UserObjects/ConstantViewFactorSurfaceRadiation

!syntax inputs /UserObjects/ConstantViewFactorSurfaceRadiation

!syntax children /UserObjects/ConstantViewFactorSurfaceRadiation
