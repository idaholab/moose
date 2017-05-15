#TabulatedFluidProperties
!description /Modules/FluidProperties/TabulatedFluidProperties

The TabulatedFluidProperties UserObject calculates the density, internal energy
and enthalpy of the fluid using bicubic spline interpolation on data provided
in a text file. All other properties are calculated using a provided FluidProperties
UserObject.

Property values are read from a file containing keywords followed by data.
Monotonically increasing values of pressure and temperature must be included in
the data file, specifying the phase space where tabulated fluid properties will
be defined. An error is thrown if either temperature or pressure data is not
included or not monotonic, and an error is also thrown if this UserObject is
requested to provide a fluid property outside this phase space.

This class is intended to be used when complicated formulations for density,
internal energy or enthalpy are required, which can be computationally expensive.
This is particularly the case where the fluid equation of state is based on a
Helmholtz free energy that is a function of density and temperature, like that
used in CO2FluidProperties. In this case, density must be solved iteratively using
pressure and temperature, which increases the computational burden.

In these cases, using an interpolation of the tabulated fluid properties can
significantly reduce the computational time for computing density, internal energy,
and enthalpy.

##File format
The expected file format for the tabulated fluid properties is now described.
Lines beginning with # are ignored, so comments can be included.
Keywords 'pressure' and 'temperature' must be included, each followed by numerical data
that increases monotonically. A blank line signifies the end of the data for the
preceding keyword.

Fluid properties for density, internal energy, and enthalpy can be included, with
the keyword 'density', 'internal_energy', or 'enthalpy' followed by data that cycles
first by temperature then pressure. If any of these properties are not supplied,
this UserObject will generate it using the pressure and temperature values provided.
An error is thrown if an incorrect number of property values has been supplied.

If no tabulated fluid property data file exists, then data for density, internal energy
and enthalpy will be generated using the pressure and temperature ranges specified
in the input file at the beginning of the simulation.

This tabulated data will be written to file in the correct format,
enabling suitable data files to be created for future use. There is an upfront
computational expense required for this initial data generation, depending on the
required number of pressure and temperature points. However, provided that the
number of data points required to generate the tabulated data is smaller than the
number of times the property members in the FluidProperties UserObject are used,
the initial time to generate the data and the subsequent interpolation time can be much
less than using the original FluidProperties UserObject.

Density, internal_energy and enthalpy and their derivatives with respect to pressure and
temperature are always calculated using bicubic spline interpolation, while all
remaining fluid properties are calculated using the provided FluidProperties UserObject.

A function to write generated data to file using the correct format is provided
to allow suitable files of fluid property data to be generated using the FluidProperties
module UserObjects.

!parameters /Modules/FluidProperties/TabulatedFluidProperties

!inputfiles /Modules/FluidProperties/TabulatedFluidProperties

!childobjects /Modules/FluidProperties/TabulatedFluidProperties
