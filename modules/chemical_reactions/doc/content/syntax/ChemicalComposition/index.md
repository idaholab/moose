# ChemicalComposition Action System

!alert! note title=For Thermochimica module
This action is designed for use with thermochemistry library Thermochimica [!cite](piro2013). Check out the corresponding submodule by running `git submodule update --init --checkout modules/chemical_reactions/contrib/thermochimica`.
!alert-end!

## Description

The `ChemicalComposition` action simplifies the initialization of variables for
thermochemical modeling and the thermodynamic material model. It initializes auxiliary variables
based on the `elements` parameter (recommended in the  `GlobalParameters` block). The
created variables have names of chemical elements used in the model and can be passed
to other objects in the simulation.

## Example Input File Syntax

An example of the `ChemicalComposition` block is shown in [chemact:modfile]. It will create two aux variables `Mo` and `Ru`, initialize their values over the entire model to values specified in file `ic.csv`, and read in thermodynamic material model defined in file `Kaye_NobleMetals.dat`.

!listing modules/chemical_reactions/test/tests/thermochimica/csv_ic.i id=chemact:modfile block=GlobalParams ChemicalComposition

[!param](/ChemicalComposition/initial_values) can be used for initialization of the
variables specified in the vector `elements` of the block `[GlobalParams]`.
The initialization file is expected to be in comma-separated value(csv) format as
shown in [chemact:inifile]. The first line of the file is ignored.

!listing modules/chemical_reactions/test/tests/thermochimica/ic.csv id=chemact:inifile caption=Initialization of chemical elements variables from a file.

### Subblocks

The subblocks of the `ChemicalComposition` action are what triggers MOOSE objects to be built. If no block restrictions apply to the constructed [`ThermochimicaElementData`](ThermochimicaElementData.md) or [`ThermochimicaNodalData`](ThermochimicaNodalData.md), a single subblock can be used.

If different user objects are needed, multiple subblocks with subdomain restrictions can be used.

!listing modules/chemical_reactions/test/tests/thermochimica/MoRu_subblock.i block=ChemicalComposition

Parameters supplied at the `[ChemicalComposition]` level act as defaults for the action subblocks but can be overridden by specifying the parameter within the subblock.

!syntax parameters /ChemicalComposition

!bibtex bibliography
