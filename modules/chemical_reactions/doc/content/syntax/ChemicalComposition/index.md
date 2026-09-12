# ChemicalComposition Action System

!alert! note title=For Use with Thermochimica
This action is designed for use with the Thermochimica thermochemistry library [!cite](piro2013).
Initialize the corresponding submodule with `git submodule update --init --checkout
modules/chemical_reactions/contrib/thermochimica`.
!alert-end!

## Description

The `ChemicalComposition` action system creates the element and thermochemical output variables
and configures equilibrium calculations at nodes or elements. Thermochemical quantities are
selected with named blocks under `[Outputs]`; the leaf-block names become the scalar auxiliary
variable names. See [ChemicalCompositionAction.md] for the available output families and execution
options.

## Example Input File Syntax

The following example creates `Mo` and `Ru` auxiliary variables, initializes their values from
`ic.csv`, and uses `Kaye_NobleMetals.dat` as the thermodynamic database:

!listing modules/chemical_reactions/test/tests/thermochimica/csv_ic.i id=chemical-composition-csv block=GlobalParams ChemicalComposition

The [!param](/ChemicalComposition/initial_composition_file) is a comma-separated file. Its first
line is treated as a header, and each remaining line specifies the initial value of one configured
element:

!listing modules/chemical_reactions/test/tests/thermochimica/ic.csv id=chemical-composition-csv-file caption=Initial element compositions read from a CSV file.

## Subblocks

Parameters on the parent `[ChemicalComposition]` block provide defaults for each named subblock.
A subblock may override those defaults. Multiple subblocks can use different databases or
evaluation locations when their [!param](/ChemicalComposition/block) restrictions do not overlap.

!listing modules/chemical_reactions/test/tests/thermochimica/MoRu_subblock.i block=ChemicalComposition

## Thermochemical Output Blocks

The following example uses each typed output family. The `variable` parameter may override a
leaf-block variable name, as shown for `bcc_mo_element_amount`.

!listing modules/chemical_reactions/test/tests/thermochimica/typed_outputs.i block=ChemicalComposition

!syntax parameters /ChemicalComposition

!bibtex bibliography
