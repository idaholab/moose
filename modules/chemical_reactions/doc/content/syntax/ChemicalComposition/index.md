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

!syntax parameters /ChemicalComposition

An example of the `ChemicalComposition` block is shown in [chemact:modfile]

!listing modules/chemical_reactions/test/tests/thermochimica/csv_ic.i id=chemact:modfile block=GlobalParams ChemicalComposition

[chemact:modfile] will create two aux variables `Mo` and `Ru`,
initialize their values over the entire model to values
specified in file `ic.csv`, and read in thermodynamic material model
defined in file `Kaye_NobleMetals.dat`.

[!param](/ChemicalComposition/ChemicalCompositionAction/initial_values) can be used for initialization of the
variables specified in the vector `elements` of the block `[GlobalParams]`.
The initialization file is expected to be in comma-separated value(csv) format as
shown in [chemact:inifile]. The first line of the file is ignored.

!listing modules/chemical_reactions/test/tests/thermochimica/ic.csv id=chemact:inifile caption=Initialization of chemical elements variables from a file.

!bibtex bibliography
