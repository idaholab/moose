# ChemicalComposition

!alert note title=For Use with Thermochimica
This action is designed for use with the thermochemistry library Thermochimica [!cite](piro2013). Check out the corresponding submodule by running `git submodule update --init --checkout modules/chemical_reactions/contrib/thermochimica`.

## Description

The `ChemicalComposition` action is used to initiate the framework for thermochemical calculations using the thermodynamics solver [*Thermochimica*](https://github.com/ORNL-CEES/thermochimica). The action creates variables needed for the analysis, reads the thermodynamic model for material system, and sets the units for temperature, pressure, and elemental amounts. It can optionally initiate the amounts of elements used in the model.

The `ChemicalComposition` action is also used to specify lists of chemical phases and chemical species for which concentrations should be output at each timestep. The phase and species names must match those in the data file specified. The `ChemicalComposition` action creates all AuxVariables needed for output of phase concentration data, chemical potentials, vapor pressures, etc. The list of phases is specified in the `output_phases` variable, and the list of species is specified in `output_species`. The species must be specified in the convention `phase_name:species_name` so that the desired species in the desired phase is uniquely identified. Alternatively, users can specify `ALL` to output all species present in the specified thermodynamic database. Other desired outputs such as element potentials, vapor pressures of species in gas phase and moles of elements in specified phases can be specified using `output_element_potentials`, `output_vapor_pressures`, and `output_element_phases` respectively. The `ChemicalComposition` action creates a UserObject derived from [`ThermochimicaNodalData`](/userobjects/ThermochimicaNodalData.md) which runs Thermochimica at each node and stores the output in the created aux variables.

!alert note
The `ALL` option circumvents the need for users to explicitly state each input element and the output variables. When used for specifying the input `elements`, all the chemical elements present in the thermodynamic database are parsed and used to create the required MOOSE variables. The users must however be careful to specify the proper values of these elements either directly in the input file (for example, as done using ICs in `MoRu.i`) or using CSV inputs (such as in `csv.i`). When the `ALL` option is used to specify outputs, all phases / species / element potentials / etc. that are present in the thermodynamic database and which can exist with the given combination of input elements will be outputed. The users must be aware that, depending on the size of the thermodynamic database, this can lead to a large number of such variables being written to the output files leading to slowdown of both the simulations as well as higher memory use.

## Variables for Chemical Elements

Chemical element variables to be used and initiated in the model using in `ChemicalComposition` action are specified using the block `[GlobalParams]`, and vector `elements` as in [chemact:globpar]:

!listing id=chemact:globpar caption=Chemical elements to be used in the model
[GlobalParams]
  elements = 'Mo Ru'
[]

The vector `elements` will be passed to the `ChemicalComposition` action and used to generate scalar variables, (e.g. `Mo`, and `Ru`) of the chemical elements contained in the vector.

!alert note
The names of scalar variables listed in `elements` vector and created using `ChemicalComposition` action should not be used for other variables in the model, e.g. the model should not define variables `Mo`, or `Ru` in the `[Variables]` or `[AuxVariables]` blocks.

The element composition scalar variables can be initialized in `ChemicalComposition` action, or using `[Functions]` block combined with other MOOSE capabilities.

## Variables for Chemical Phases, Species, and Element Chemical Potentials

Chemical phase and species variables, as well as chemical potentials of elements, to be used and initiated in the model using in `ChemicalComposition` action are specified using the block `[GlobalParams]`, and vector `output_phases` as in [phaseact:globpar]:

!listing modules/chemical_reactions/test/tests/thermochimica/MoRu.i id=phaseact:globpar start=GlobalParams end=[] include-end=True caption=Chemical phases species to be used in the model, and list of desired output variables.

The vectors `output_phases`, `output_species`, `output_element_potentials`, `output_vapor_pressures`, and `output_element_phases` will be passed to the `ChemicalComposition` action and used to generate scalar variables, (e.g. `BCCN` and `BCCN:Mo`) for the chemical phases and species contained in the vectors. The `output_phases`, `output_species`, `output_element_potentials`, `output_vapor_pressures`, and `output_element_phases` vectors are optional coupled variables for `ThermochimicaNodalData`, so this vector and use of the `ChemicalComposition` action are optional when using UserObjects derived from this class.

!alert note
The names of scalar variables listed in `phases` vector and created using `ChemicalComposition` action should not be used for other variables in the model, e.g. the model should not define variable `BCCN` in the `[Variables]` or `[AuxVariables]` blocks.

!alert note
The format for entries in `output_species` must be `phase_name:species_name` or `ALL`, and both must match the specified thermodynamic database.

!alert note
The format for entries in `output_element_potentials` must be `any_string:element_name`  or `ALL`.

!alert note
The format for entries in `output_vapor_pressures` must be `any_string:gas_phase_name:species_name`  or `ALL`. The phase name must correspond to the gas phase in the thermodynamic database.

!alert note
The format for entries in `output_element_phases` must be `any_string:phase_name:element_name`  or `ALL`.

The variables created by this action will be used by the UserObject `ThermochimicaNodalData` (or, more likely, a derived class specific to the application) to generate arrays in which to store the indices of the requested phases in Thermochimica calculation results, and to store the corresponding concentration values.

The elemental composition is used in `Thermochimica` calculations, e.g `ThermochimicaNodalData` UserObject, that expect an `elements` vector.

This `ChemicalComposition` action syntax is shown on the
[ChemicalComposition action](/ChemicalComposition/index.md) action
system page.

!bibtex bibliography
