# SalineMoltenSaltFluidProperties

!syntax description /FluidProperties/SalineMoltenSaltFluidProperties

The mole fractions of the various elements / molecules present in the salt must sum to one.
The following properties can be computed:

- density
- isobaric specific heat
- enthalpy
- viscosity
- thermal diffusivity

!alert! note
Saline must be installed alongside MOOSE using the `contrib/saline` submodule in the Fluid Properties
module to be able to use this fluid property object. To initialize/download the Saline submodule:

```bash
cd <directory where moose is downloaded>
git submodule update --init --checkout modules/fluid_properties/contrib/saline
```

!alert-end!

## Example input syntax

In this example, the fluid properties for a `LiF-NaF-KF` salt mixture are computed by a
`SalineMoltenSaltFluidProperties`.

!listing modules/fluid_properties/test/tests/saline/test.i block=FluidProperties

It leverages the salt property input data in the `saline_custom.prp` file below.
The data is sorted in columns. The first column is the name of the salt, then its id, then
a number of thermo-physical properties with additional metadata such as the range of validity,
the uncertainty and a short name for the reference publication providing the data.

!alert note
More details on working with Saline can be found in this [reference](https://info.ornl.gov/sites/publications/Files/Pub167853.pdf).

!listing modules/fluid_properties/test/tests/saline/saline_custom.prp

!syntax parameters /FluidProperties/SalineMoltenSaltFluidProperties

!syntax inputs /FluidProperties/SalineMoltenSaltFluidProperties

!syntax children /FluidProperties/SalineMoltenSaltFluidProperties

!bibtex bibliography
