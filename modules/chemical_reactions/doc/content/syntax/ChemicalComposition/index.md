# ChemicalComposition Action System

!alert! note title=For Thermochemica module
This action is designed for use with Thermochimica library.
!alert-end!

## Description

The `ChemicalComposition` action simplifies the initiation of varaibles for thermochemical modeling and thermodynamic material model. It iniitates auxilliary or regular variables based on elements varaible in GlobalParameters block. The created variables have names of chemical elements used in the model and can be passed to other objects in the simulation.

!syntax parameters /ChemicalComposition

An example of the `ChemicalComposition` block is shown in [chemact:modfile]

!listing id=chemact:modfile caption=Initialization of chemical elements to be used in the model
[GlobalParams]
  elements = 'O U Pu'
[]
 
[ChemicalComposition]
  var_type = base
  initial_values = ic.csv
  thermofile = Pu_U_O_CEA.dat
[]

[chemact:modfile] will create three `base` type variables `O`, `U`,
and `Pu`, initial their iniital values over the entire model to values
specified in file `ic.csv`, and read in thermodynamic material model
defined in file `Pu_U_O_CEA.dat`.

Parameter `initial_values' can be used for initialization of the variables specified in
the vector `elements`  of the block `[GlobalParams]`.
The iniitalization file is expected to be in comma-separated value(csv) format as shown in [chemact:inifile]. The first line of the file is ignored.

!listing id=chemact:inifile caption=Initialization of chemical elements variables from a file.
element,initial condition
O,8.4030E+03
U,3.6971E+03
Pu,5.7050E+01


