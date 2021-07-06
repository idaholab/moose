# AbaqusUExternalDB

!syntax description /UserObjects/AbaqusUExternalDB

## Description

The `AbaqusUExternalDB` user object  is used to execute an _Abaqus UEXTERNALDB_
user function at various stages of the simulation. The functions can be coded in
Fortran (`.f` and `.f90` file extensions) or C/C++  (`.c` and `.C` file
extensions) and must be located in the `plugins` directory of the app.

Data exchange between UEXTERNALDB and UMAT may require to put both functions in
the same translation unit (source file).

A description of the input and output parameters of the UEXTERNALDB user subroutines
can be found in the Abaqus user manual.

!alert note
MOOSE has no corresponding concept of an Abaqus simulation "step". MOOSE time steps
correspond to Abaqus "increments". For now the Abaqus step number passed to the
UEXTERNALDB routine is always zero.

!syntax parameters /UserObjects/AbaqusUExternalDB

!syntax inputs /UserObjects/AbaqusUExternalDB

!syntax children /UserObjects/AbaqusUExternalDB
