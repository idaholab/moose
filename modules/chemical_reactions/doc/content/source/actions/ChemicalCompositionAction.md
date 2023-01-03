# ChemicalComposition

!alert note title=For Use with Thermochimica
This action is designed for use with the thermochemistry library Thermochimica [!cite](piro2013). Check out the corresponding submodule by running `git submodule update --init --checkout modules/chemical_reactions/contrib/thermochimica`.

## Description

The `ChemicalComposition` action is used to initiate the framework for thermochemical calculations using the thermodynamics solver [_Thermochimica_](https://github.com/ORNL-CEES/thermochimica). The action creates variables needed for the analysis, reads the thermodynamic model for material system, and sets the units for temperature, pressure, and elemental amounts. It can optionally initiate the amounts of elements used in the model.

The `ChemicalComposition` action is also used to specify lists of chemical phases and chemical species for which concentrations should be output at each timestep. The phase and species names must match those in the data file specified. The `ChemicalComposition` action creates all AuxVariables and AuxKernels needed for output of phase concentration data. The list of phases is specified in the `output_phases` variable, and the list of species is specified in `output_species`. The species must be specified in the convention `phase_name:species_name` so that the desired species in the desired phase is uniquely identified. A combination of an AuxKernel derived from `ThermochimicaAux` and a UserObject derived from `ThermochimicaNodalData` can then be used to store the concentrations of these species in their respective phases.

## Variables for Chemical Elements

Chemical element variables to be used and initiated in the model using in `ChemicalComposition` action are specified using the block `[GlobalParams]`, and vector `elements` as in [chemact:globpar]:

!listing id=chemact:globpar caption=Chemical elements to be used in the model
[GlobalParams]
  elements = 'O U Pu'
[]

The vector `elements` will be passed to the `ChemicalComposition` action and used to generate scalar variables, (e.g. `O`, `U`, and `Pu`) of the chemical elements contained in the vector.

!alert note
The names of scalar variables listed in `elements` vector and created using `ChemicalComposition` action should not be used for other variables in the model, e.g. the model should not define variables `O`, `U`, or `Pu` in the `[Variables]` or `[AuxVariables]` blocks.

The element composition scalar variables can be initialized in `ChemicalComposition` action, or using `[Functions]` block combined with other MOOSE capabilities.

## Variables for Chemical Phases, Species, and Element Chemical Potentials

Chemical phase and species variables, as well as chemical potentials of elements, to be used and initiated in the model using in `ChemicalComposition` action are specified using the block `[GlobalParams]`, and vector `output_phases` as in [phaseact:globpar]:

!listing id=phaseact:globpar caption=Chemical phases species to be used in the model, and list of output phases for which concentration data is desired.
[GlobalParams]
  elements = 'Zr U'
  output_phases = 'ORTHORHOMBIC_A20 TETRAGONAL_U BCC_A2_1 BCC_A2_2'
  output_species = 'ORTHORHOMBIC_A20:ZR TETRAGONAL_U:ZR BCC_A2_1:ZR BCC_A2_2:ZR'
  element_potentials = 'cp:Zr cp:U'
[]

The vectors `output_phases`, `output_species`, and `element_potentials` will be passed to the `ChemicalComposition` action and used to generate scalar variables, (e.g. `ORTHORHOMBIC_A20` and `TETRAGONAL_U:ZR`) for the chemical phases and species contained in the vectors. The `output_phases`, `output_species`, and `element_potentials` vectors are optional coupled variables for `ThermochimicaNodalData`, so this vector and use of the `ChemicalComposition` action are optional when using UserObjects derived from this class.

!alert note
The names of scalar variables listed in `phases` vector and created using `ChemicalComposition` action should not be used for other variables in the model, e.g. the model should not define variable `ORTHORHOMBIC_A20` in the `[Variables]` or `[AuxVariables]` blocks.

!alert note
The format for entries in `output_species` must be `phase_name:species_name`, and both must match the specified thermodynamic database.

!alert note
The format for entries in `element_potentials` must be `any_string:element_name`.

The variables created by this action will be used by the UserObject `ThermochimicaNodalData` (or, more likely, a derived class specific to the application) to generate arrays in which to store the indices of the requested phases in Thermochimica calculation results, and later by an AuxKernel derived from `ThermochimicaAux` to store the corresponding concentration values. Users intending to implement new classes that make use of `Thermochimica` and desire output phase concentration data should examine `ThermochimicaUO2XTransportAux` to ensure updating of these variables.

The elemental composition is used in `Thermochimica` calculations, e.g `AuxKernels`, that expect an `elements` vector.

This `ChemicalComposition` action syntax is shown on the
[ChemicalComposition action](/ChemicalComposition/index.md) action
system page.

!bibtex bibliography
