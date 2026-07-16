# ChemicalComposition Action

!alert note title=For Use with Thermochimica
This action requires the Thermochimica submodule and the `thermochimica` capability.

## Description

The `ChemicalComposition` action configures equilibrium calculations using the
[Thermochimica](https://github.com/ORNL-CEES/thermochimica) thermochemistry library. It creates
scalar auxiliary variables for the selected elements and thermochemical outputs, validates the
requested quantities against the [!param](/ChemicalComposition/thermodynamic_database), and
evaluates Thermochimica at each selected node or element.

The [!param](/ChemicalComposition/temperature) is required and may be supplied by a variable or a
constant. The [!param](/ChemicalComposition/pressure) may also be a variable or a constant and
defaults to 1. The units of these values and the elemental compositions are specified with
[!param](/ChemicalComposition/temperature_unit), [!param](/ChemicalComposition/pressure_unit), and
[!param](/ChemicalComposition/composition_unit), respectively.

The [!param](/ChemicalComposition/evaluation_location) determines where equilibrium is evaluated.
The default, `nodal`, creates Lagrange auxiliary variables. The `elemental` option creates
elemental monomial auxiliary variables and is suitable for finite-volume calculations.

## Element Variables and Initialization

The [!param](/ChemicalComposition/elements) parameter lists the chemical elements used in the
equilibrium calculation. The action creates an auxiliary variable with the same name as each
element. These variables may be initialized with standard MOOSE initial conditions or with the
[!param](/ChemicalComposition/initial_composition_file). The CSV file must contain a header followed
by one `element,value` pair per line.

Specifying `ALL` for [!param](/ChemicalComposition/elements) creates variables for every element in
the thermodynamic database. Every created element variable must be initialized with a physically
meaningful composition before Thermochimica is evaluated.

## Thermochemical Outputs

Typed output blocks select database phases, species, and elements without encoding those
selections into the auxiliary variable name. Each named leaf block creates one scalar auxiliary
variable. The leaf-block name is used as the variable name unless the `variable` parameter is
provided.

!table id=chemical-composition-typed-outputs caption=Typed thermochemical output blocks.
| Block under `[Outputs]` | Required parameters | Quantity |
| :- | :- | :- |
| `[Phases]` | `phase` | Phase moles or system mole fraction |
| `[Species]` | `phase`, `species` | Species moles or mole fraction within its phase |
| `[ElementPotentials]` | `element` | Element chemical potential |
| `[VaporPressures]` | `phase`, `species` | Species partial pressure in a gas phase |
| `[ElementDistribution]` | `phase`, `element` | Element moles or fraction distributed to a phase |

The `unit` parameter in `[Phases]` and `[Species]` selects `moles` or `mole_fraction`. A phase mole
fraction is the phase moles divided by the total moles in all phases. A species mole fraction is the
species mole fraction within its selected phase.

For `[ElementDistribution]`, `unit = moles` reports the moles of the selected element in the phase,
while `unit = fraction` reports the fraction of that element distributed to the phase. The latter is
the element moles in the selected phase divided by the element moles summed over all phases.

The following example creates output variables including `hcp_amount`, `bcc_phase_fraction`,
`hcp_phase_fraction`, `hcp_mo_fraction`, `hcp_mo_moles`, `mo_chemical_potential`,
`mo_vapor_pressure`, `hcp_mo_element_amount`, and element distribution fractions:

!listing modules/chemical_reactions/test/tests/thermochimica/typed_outputs.i block=ChemicalComposition

!alert note
Element and output variable names must be unique and must not also be declared in `[Variables]` or
`[AuxVariables]`.

## Deprecated Output Parameters

The flat output parameters remain available for compatibility and support `ALL` expansion. They
are deprecated in favor of typed output blocks. Their selectors generate the following legacy
variable names:

!table id=chemical-composition-output-names caption=Selectors and generated auxiliary variable names.
| Parameter | Selector format | Generated variable |
| :- | :- | :- |
| [!param](/ChemicalComposition/output_phases) | `phase` | `phase` |
| [!param](/ChemicalComposition/output_species) | `phase:species` | `phase:species` |
| [!param](/ChemicalComposition/output_element_potentials) | `element` | `mu:element` |
| [!param](/ChemicalComposition/output_vapor_pressures) | `gas_phase:species` | `vp:gas_phase:species` |
| [!param](/ChemicalComposition/output_element_phases) | `phase:element` | `ep:phase:element` |

The [!param](/ChemicalComposition/species_output_unit) selects whether species quantities are
reported as `moles` or `mole_fraction`. Each output selector also accepts `ALL`. For a large
database, `ALL` may create many auxiliary variables and substantially increase memory use and
output-file size.

## Batching and Warm Starts

The action performs an exact Thermochimica equilibrium solve for every selected node or element.
The [!param](/ChemicalComposition/batch_size) controls how many states are sent in each request and
defaults to 32. Setting it to 1 is useful for debugging and performance comparisons.

The [!param](/ChemicalComposition/warm_start) controls the initial guess for each exact solve:

- `previous_solve` is the default and reuses the preceding result from the same worker.
- `previous_timestep` stores a result for each node or element and generally uses more memory.
- `none` disables warm starts and does not allocate per-entity reinitialization storage.

## Example Input Syntax

The following example requests phase amounts, species concentrations, element potentials, vapor
pressures, and element amounts in phases with typed output blocks:

!listing modules/chemical_reactions/test/tests/thermochimica/typed_outputs.i block=ChemicalComposition

Common parameters can be specified on the parent `[ChemicalComposition]` block and overridden in
named subblocks. See the [ChemicalComposition syntax](syntax/ChemicalComposition/index.md) for an
example using different databases on disjoint mesh blocks.

!syntax parameters /ChemicalComposition

!bibtex bibliography
