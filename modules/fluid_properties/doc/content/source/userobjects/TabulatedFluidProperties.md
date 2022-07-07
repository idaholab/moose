# TabulatedFluidProperties

!syntax description /Modules/FluidProperties/TabulatedFluidProperties

The TabulatedFluidProperties UserObject calculates fluid properties using either bilinear or bicubic interpolation
of data provided in a text file.
Note, Bilinear Interpolation uses two vectors and a ColumnMajorMatrix, while Bicubic Interpolation uses two vectors and a vector of a vector.

Property values are read from a CSV file containing property data.  Monotonically increasing values
of pressure and temperature must be included in the data file, specifying the phase space where
tabulated fluid properties will be defined. An error is thrown if either temperature or pressure data
is not included or not monotonic, and an error is also thrown if this UserObject is requested to
provide a fluid property outside this phase space.

This class is intended to be used when complicated formulations for fluid properties (such as
density or internal energy) are required, which can be computationally expensive.  This is particularly
the case when the fluid equation of state is based on a Helmholtz free energy that is a function of
density and temperature, like that used in [CO2FluidProperties](/CO2FluidProperties.md). In this example,
density must be solved iteratively using pressure and temperature, which increases the computational burden.

Using an interpolation of tabulated fluid properties can significantly reduce the computational time
for computing fluid properties defined using complex equations of state, which may reduce the overall
computational cost dramatically, especially if fluid properties are calculated a large number of times.

## File format

The expected file format for the tabulated fluid properties is now described.  The first line must be
the header containing the required column names *pressure* and *temperature*, and also any number of
the fluid properties that TabulatedFluidProperties understands. These are: *density*, *enthalpy*,
*internal_energy*, *viscosity*, *k* (the thermal conductivity), *cp* (the isobaric specific heat
capacity), *cv* (the isochoric specific heat capacity), and *entropy*.

!alert note
The order is not important, although having pressure and temperature first makes the data easier
for a human to read.

The data in the pressure and temperature columns *must* be monotonically increasing. This file format
does require duplication of the pressure and temperature data - each pressure value must be included
num_T times, while each temperature value is repeated num_p times, where num_T and num_p are the
number of temperature and pressure points, respectively. This class will check that the required
number of data points have been entered (num_T * num_p).

An example of a valid fluid properties file is provided below:

```text
pressure, temperature,   density, enthalpy, internal_energy
  200000,         275,   3.90056,   -21487,        -72761.7
  200000,         277,   3.86573, -19495.4,        -71232.0
  200000,         280,   3.83155, -17499.1,        -69697.3
  300000,         275,   6.07273, -22728.3,        -73626.5
  300000,         277,   6.01721, -20711.5,        -72079.3
  300000,         280,   5.96277, -18691.0,        -70527.7
```

and so on.

### Alternative variable sets

Alternatively, from the input internal energy and volume TFP can determine various parameters (p,T,g,c,cp,cv,k, and mu from (v,e)).

We convert to (v,e) by creating (v,e) and (v,h) grids. The grids are created by filling points using volume[j]=_v_min + j * dv for specific volume and
energy[j]=_e_min + j * de for j=0 to number of points.
These grids are then filled and (p,T) data are matched to the grid points. The h and e grids use the same number of grid points.

The bounds are determined by the maximums and minimums of the input parameters.

## Using TabulatedFluidProperties

### Reading from an existing file

Consider the example where TabulatedFluidProperties is used to reduce the cost of calculating
CO$_2$ fluid properties. In this example, a file containing the tabulated fluid properties, named
`fluid_properties.csv` is provided. All properties listed in this file will be calculated using
either Bilinear or Bicubic interpolation (the interpolation type is to be specified in the input file), while all remaining properties provided by the `FluidProperties` interface will be
calculated using a [CO2FluidProperties](/CO2FluidProperties.md) UserObject.

Bilinear Interpolation and Bicubic Interpolation are sub classes of Bidimensional Interpolation. The TabulatedFluidProperties inherits Bilinear or Bicubic Interpolation depending on the type specified in the input file.

The input file syntax necessary to achieve this is

!listing modules/fluid_properties/test/tests/tabulated/tabulated.i block=Modules
or
!listing modules/fluid_properties/test/tests/tabulated/tabulated_v_e.i block=Modules

The "tabulated_v_e.i" input file is to be used if ICs are v and e. If ICs are (p,T), "tabulated.i" should be used.

### Writing data file

If no tabulated fluid property data file exists, then data for the properties specified in the
input file parameter *interpolated_properties* will be generated using the pressure and temperature
ranges specified in the input file at the beginning of the simulation.

For example, if we wish to generate a file containing tabulated properties for CO$_2$ density, enthalpy
and viscosity for $300 \mathrm{K} \le T \le 400 \mathrm{K}$ and $1 \mathrm{MPa} \le p \le 10 \mathrm{MPa}$,
divided into 50 and 100 equal points, respectively, then the input file syntax necessary is

```
[Modules]
  [./FluidProperties]
    [./co2]
      type = CO2FluidProperties
    [../]
    [./tabulated]
      type = TabulatedBi___FluidProperties
      fp = co2
      fluid_property_file = fluid_properties.csv
      interpolated_properties = 'density enthalpy viscosity'
      temperature_min = 300
      temperature_max = 400
      pressure_min = 1e6
      pressure_max = 10e6
      num_T = 50
      num_p = 100
    [../]
  []
[]
```
Note that Bi___ should be replaced with Bicubic or Bilinear depending on desired the Interpolation method

This tabulated data will be written to file in the correct format, enabling suitable data files to be
created for future use. There is an upfront computational expense required for this initial data
generation, depending on the required number of pressure and temperature points. However, provided
that the number of data points required to generate the tabulated data is smaller than the number of
times the property members in the FluidProperties UserObject are used, the initial time to generate
the data and the subsequent interpolation time can be much less than using the original
FluidProperties UserObject.

!alert note
All fluid properties read from a file or specified in the input file (and their derivatives with
respect to pressure and temperature) will be calculated using bicubic or bilinear interpolation, while all
remaining fluid properties will be calculated using the provided FluidProperties UserObject.

!syntax parameters /Modules/FluidProperties/TabulatedFluidProperties

!syntax inputs /Modules/FluidProperties/TabulatedFluidProperties

!syntax children /Modules/FluidProperties/TabulatedFluidProperties
