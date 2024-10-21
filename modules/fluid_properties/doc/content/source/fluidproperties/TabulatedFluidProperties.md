# TabulatedFluidProperties

Single phase fluid properties computed using bi-dimensional interpolation of tabulated values.

Property values are read from a CSV file containing property data. Monotonically increasing values
of pressure and temperature must be included in the data file, specifying the phase space where
tabulated fluid properties will be defined. An error is thrown if either temperature or pressure data
is not included or not monotonic, and an error is also thrown if this object is requested to
provide a fluid property outside this phase space.

This class is intended to be used when complicated formulations for fluid properties (such as
density or internal energy) are required, which can be computationally expensive.  This is particularly
the case when the fluid equation of state is based on a Helmholtz free energy that is a function of
density and temperature, like that used in [CO2FluidProperties](/CO2FluidProperties.md). In this example,
density must be solved iteratively using pressure and temperature, which increases the computational burden.

Using an interpolation of tabulated fluid properties can significantly reduce the computational time
for computing fluid properties defined using complex equations of state, which may reduce the overall
computational cost dramatically, especially if fluid properties are calculated a large number of times.

!alert note
`TabulatedFluidProperties` is a base class and may not be used. A derived class, specifying an interpolation
method to use to interpolate between the tabulated data, must be used instead.
Currently only [bicubic tabular interpolation](TabulatedBicubicFluidProperties.md) is implemented.
Bilinear interpolation is a work in progress.

## File format id=format

The expected file format for the tabulated fluid properties is now described.  The first line must be
the header containing the required column names *pressure* and *temperature*, and also any number of
the fluid properties that `TabulatedFluidProperties` understands. The exact names used are:

- *density*
- *enthalpy*
- *v* (specific_volume)
- *internal_energy*
- *viscosity*
- *k* (the thermal conductivity)
- *g* (gibbs free energy)
- *cp* (the isobaric specific heat capacity)
- *cv* (the isochoric specific heat capacity)
- *c* (speed of sound in fluid)
- *entropy*


!alert note
The order is not important, although having pressure and temperature first makes the data easier for a human to read.

The data in the pressure and temperature columns *must* be monotonically increasing. This file format
does require duplication of the pressure and temperature data - each pressure value must be included
[!param](/FluidProperties/TabulatedFluidProperties/num_T) times, while each temperature value is repeated
[!param](/FluidProperties/TabulatedFluidProperties/num_p) times, where
[!param](/FluidProperties/TabulatedFluidProperties/num_T) and [!param](/FluidProperties/TabulatedFluidProperties/num_p) are the
number of temperature and pressure points, respectively. This class will check that the required
number of data points have been entered
([!param](/FluidProperties/TabulatedFluidProperties/num_T) * [!param](/FluidProperties/TabulatedFluidProperties/num_p)).

An example of a valid fluid properties file, with two pressure points and three temperature points, is provided below:

```text
pressure, temperature,   density, enthalpy, internal_energy
  200000,         275,   3.90056,   -21487,        -72761.7
  200000,         277,   3.86573, -19495.4,        -71232.0
  200000,         280,   3.83155, -17499.1,        -69697.3
  300000,         275,   6.07273, -22728.3,        -73626.5
  300000,         277,   6.01721, -20711.5,        -72079.3
  300000,         280,   5.96277, -18691.0,        -70527.7
```

## Using TabulatedFluidProperties

### Reading from an existing file

Consider the example where a `TabulatedFluidProperties` object is used to reduce the cost of calculating
CO$_2$ fluid properties. In this example, a file containing the tabulated fluid properties, named
`fluid_properties.csv` is provided. All properties listed in this file will be calculated using
either Bilinear or Bicubic interpolation (the interpolation type is to be specified in the input file),
while all remaining properties provided by the `FluidProperties` interface will be
calculated using a [CO2FluidProperties](/CO2FluidProperties.md) object.

The input file syntax necessary to achieve this with a (pressure, temperature) variable set is shown below.
A [TabulatedBicubicFluidProperties.md] is used.

!listing modules/fluid_properties/test/tests/tabulated/tabulated.i block=FluidProperties

With a (specific volume, specific energy) variable set, the syntax shown in the example file below may be used:

!listing modules/fluid_properties/test/tests/tabulated/tabulated_v_e.i block=FluidProperties


### Writing data file

The `TabulatedFluidProperties`-derived classes can write a file containing the data for the properties specified in the
input file parameter *interpolated_properties*. It will use the pressure and temperature
ranges specified in the input file at the beginning of the simulation.

For example, if we wish to generate a file containing tabulated properties for CO$_2$ density, enthalpy
and viscosity for $300 \mathrm{K} \le T \le 400 \mathrm{K}$ and $1 \mathrm{MPa} \le p \le 10 \mathrm{MPa}$,
divided into 50 and 100 equal points, respectively, then the input file syntax necessary is

```text
[FluidProperties]
  [co2]
    type = CO2FluidProperties
  []
  [tabulated]
    type = TabulatedBicubicFluidProperties
    fp = co2
    fluid_property_output_file = fluid_properties.csv
    interpolated_properties = 'density enthalpy viscosity'

    # Bounds of interpolation
    temperature_min = 300
    temperature_max = 400
    pressure_min = 1e6
    pressure_max = 10e6

    # Grid discretization
    num_T = 50
    num_p = 100
  []
[]
```


This tabulated data will be written to file in the correct format, enabling suitable data files to be
created for future use. There is an upfront computational expense required for this initial data
generation, depending on the required number of pressure and temperature points. However, provided
that the number of data points required to generate the tabulated data is smaller than the number of
times the property members in the FluidProperties object are used, the initial time to generate
the data and the subsequent interpolation time can be much less than using the original
FluidProperties object.

Using the  [!param](/FluidProperties/TabulatedFluidProperties/construct_pT_from_ve) parameter and the
[!param](/FluidProperties/TabulatedFluidProperties/fluid_property_output_file) parameters, a tabulation
using the (specific volume, specific internal energy) variables can be generated. The output file name
for this additional tabulation will be suffixed with `_ve.csv`.

!alert note
All fluid properties read from a file or specified in the input file (and their derivatives with
respect to pressure and temperature) will be calculated through interpolation, while all
remaining (or missing) fluid properties will be calculated using the provided `FluidProperties` object.

## Using alternative variable sets

The (pressure, temperature) variable set is not adequate for all fluid flow applications, and alternative
variable sets may be used with `TabulatedFluidProperties` objects. (specific volume (v), specific internal energy (e)),
and (specific volume, specific enthalpy (h)) are supported.

### Option 1: Creation of interpolations between variable sets

The first option is to use (pressure, temperature) variable for all fluid properties, and rely on tabulated conversions from (specific volume, specific internal energy / enthalpy) to (pressure, temperature) to compute properties.
This option is selected with the [!param](/FluidProperties/TabulatedFluidProperties/construct_pT_from_ve) and
[!param](/FluidProperties/TabulatedFluidProperties/construct_pT_from_vh) parameters.
The workflow is as follows:

- The data is read from a data file tabulated with pressure and temperature, and interpolations based on
  pressure and temperature are created for each tabulated property.

- The pressure and temperature data is converted to the alternative variable set (for example (v,e)) using [Newton's method](utils/FluidPropertiesUtils.md).
  The inversion uses the interpolations created from the tabulated data, or if available the `FluidProperties` object
  as this reduces the error.

- A grid of values for pressure and temperature is computed for the alternative variable set. This is used to create
  a tabulated interpolation from the alternative variable set to the (pressure, temperature) variable set. This process is
  described in the next section, see [TabulatedFluidProperties.md#pt_from_ve].

- When querying a fluid property using the alternative variable set, the interpolations are first used to convert
  to the (pressure, temperature) variable set (for example, computing $p(v,e)$ and $T(v,e)$). Then the fluid property
  desired is queried using this variable set (for example, $\rho(p, T)$) and returned. This can be summarized as
  $\rho(v,e) = \rho_{interpolated}(p_{interpolated}(v,e), T_{interpolated}(v,e))$


!alert note
The additional variable sets supported are $(v,e)$ and $(v,h)$. A few properties may be computed using alternative
variable sets: $e(p,\rho), T(p,\rho), T(p,h)$ and $s(h,p)$ for example.

!alert note
File data may only be read and written with the (pressure, temperature) variable set. The alternative variable set
must be either contained in the tabulation read or be computable from pressure and temperature in the
`FluidProperties` object.

### Generating (v,e) to (p,T) conversion interpolations id=pt_from_ve

As fluid properties are much more often tabulated using pressure and temperature than alternative variable
sets, the alternative variables are systematically converted to pressure and temperature to perform the
fluid property evaluations. This involves the creation of an interpolation of pressure and temperature using the
alternative variable sets. This is done in several sets, described for the $(v,e)$ set:

- A grid of $(v,e)$ data is generated. If a fluid property user object is provided, the bounds are based on
  the specified bounds on pressure and temperature : $e_{min/max} = e(p_{min/max}, T_{min/max})$, else the bounds
  are chosen from the tabulated data. The number of points in the grid in both dimensions are user-selected parameters.
  The v grid may be created using base-10 log-spacing by setting [!param](/FluidProperties/TabulatedFluidProperties/use_log_grid_v).
  The e grid may be created using base-10 log-spacing by setting [!param](/FluidProperties/TabulatedFluidProperties/use_log_grid_e).

- These bounds may not be physically realizable simultaneously. It could be that the fluid may not have both $v=v_{min}$
  and $e=e_{min}$. Part of the grid may not be physical.

- The pressure and temperature are then calculated for every point in the $(v,e)$ grid by using the
  [Newton method utilities](utils/FluidPropertiesUtils.md). Note that sometimes pressure and temperature values
  can be outside the user-defined range during this variable set inversion. when this is the case, the $(p,T)$ values are
  replaced with their respective minimum and maximum values. This only means that the interpolations will be constant
  over part of the $(v,e)$ grid, which should be outside of the range of interest.

- An interpolation object, very similar to the ones created for the other fluid properties based on $(p,T)$ data,
  is created for both variables of the alternative set. This object can then compute: $p_{interpolated}(v,e)$ and
  $T_{interpolated}(v,e)$.


!alert note
Warnings will be output when a pressure or temperature value is limited to its bound, and when an inversion from
the alternative variable set to pressure or temperature fails, often because the grid extends beyond physically
reachable values.

### Option 2: Using interpolations in (v,e) of the fluid properties

To avoid the difficulties in converting from (v,e) to (pressure, temperature) and then evaluating the properties
with (pressure, temperature), the properties can also be interpolated in (v,e). These interpolations can be created
from either another `FluidProperties` object, with the [!param](/FluidProperties/TabulatedFluidProperties/fp), or from a
(specific volume, specific internal energy) tabulation, using the [!param](/FluidProperties/TabulatedFluidProperties/fluid_property_ve_file) parameter.

Similarly as for (pressure, temperature), the list of properties to interpolate should be provided using the [!param](/FluidProperties/TabulatedFluidProperties/interpolated_properties) parameter.

The format of the tabulation is similar to the one mentioned in [TabulatedFluidProperties.md#format], except that specific volume and specific internal
energy replace pressure and temperature for the tabulation variables. Pressure and temperature will likely
instead appear as the tabulated properties, so that `temperature(v,e)` and `pressure(v,e)` can be computed directly from interpolations.

!syntax parameters /FluidProperties/TabulatedFluidProperties

!syntax inputs /FluidProperties/TabulatedFluidProperties

!syntax children /FluidProperties/TabulatedFluidProperties
