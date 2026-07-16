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

## Phase Selection

The [!param](/ChemicalComposition/excluded_phases) parameter removes named phases from every
equilibrium calculation performed by the action. Alternatively,
[!param](/ChemicalComposition/included_phases) retains only the named phases and removes every
other phase. The two parameters are mutually exclusive, and every phase name is validated against
the thermodynamic database during setup.

Phase selection is useful for evaluating constrained or metastable systems. For example, retaining
one phase and requesting its phase or system Gibbs energy provides a free-energy quantity that can
be consumed by a phase-field model:

!listing modules/chemical_reactions/test/tests/thermochimica/phase_exclusion.i block=ChemicalComposition

Outputs may only select phases that remain in the configured system. An output request for an
excluded phase is rejected during setup.

!alert note title=Fixed Phase Selection
The phase-selection list is fixed for a `ChemicalComposition` subblock throughout the simulation.
Thermochemical outputs can be coupled to phase-field equations, but the selected phase set cannot
currently be changed independently at each node or element by a phase-field variable.

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
| `[ChemicalPotentials]` | `phase` and one component selector | Species, quadruplet, or MQM pair endmember potential in J/mol |
| `[PhaseGibbsEnergies]` | `phase` | Phase Gibbs energy in J or J/mol |
| `[PhaseDrivingForces]` | `phase` | Phase driving force in J/mol-atoms |
| `[SystemGibbsEnergies]` | None | Integral system Gibbs energy in J |
| `[SystemProperties]` | `property` | Integral enthalpy, entropy, or equilibrium heat capacity |

The `unit` parameter in `[Phases]` and `[Species]` selects `moles` or `mole_fraction`. A phase mole
fraction is the phase moles divided by the total moles in all phases. A species mole fraction is the
species mole fraction within its selected phase.

For `[ElementDistribution]`, `unit = moles` reports the moles of the selected element in the phase,
while `unit = fraction` reports the fraction of that element distributed to the phase. The latter is
the element moles in the selected phase divided by the element moles summed over all phases.

Each `[ChemicalPotentials]` leaf requires exactly one of `species`, `quadruplet`, `endmember`, or
`pair`. Use `species` for a conventional solution phase and `quadruplet` for an MQM phase. The
`endmember` and `pair` selectors are equivalent for an MQM phase and report the stoichiometric
potential obtained from the system element potentials. This is not an independently minimized
partial chemical potential for the pair. All chemical potentials are reported in J/mol.

The `unit` parameter in `[PhaseGibbsEnergies]` selects `joules` or `joules_per_mole`. Phase Gibbs
energy is reported only for a stable phase; an absent phase has a value of zero. A
`[PhaseDrivingForces]` output is reported in J/mol-atoms. A negative driving force indicates that
formation of the phase is favorable, while a stable phase has a value close to zero. Each leaf in
`[SystemGibbsEnergies]` reports the integral Gibbs energy of the complete system in joules and does
not require an additional selector.

Each `[SystemProperties]` leaf selects `enthalpy`, `entropy`, or `heat_capacity` with the `property`
parameter. Enthalpy is reported in J, while entropy and heat capacity are reported in J/K. These
are integral properties of the complete configured system.

!alert warning title=Heat-Capacity Cost
Requesting any `[SystemProperties]` output enables Thermochimica's equilibrium heat-capacity
calculation, which perturbs temperature and performs additional equilibrium solves. The reported
heat capacity follows the equilibrium phase assemblage and is not a frozen-composition heat
capacity.

The following example creates output variables including `hcp_amount`, `bcc_phase_fraction`,
`hcp_phase_fraction`, `hcp_mo_fraction`, `hcp_mo_moles`, `mo_chemical_potential`,
`mo_vapor_pressure`, `hcp_mo_element_amount`, chemical potentials, phase and system Gibbs
energies, phase driving forces, and element distribution fractions:

!listing modules/chemical_reactions/test/tests/thermochimica/typed_outputs.i block=ChemicalComposition

The following example selects an MQM quadruplet chemical potential and the stoichiometric
potential of the `Fe2O3` pair endmember:

!listing modules/chemical_reactions/test/tests/thermochimica/typed_mqm.i block=ChemicalComposition/thermo/Outputs/ChemicalPotentials

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

The following example requests phase amounts, species concentrations, element and species
potentials, vapor pressures, phase and system Gibbs energies, phase driving forces, and element
amounts in phases with typed output blocks:

!listing modules/chemical_reactions/test/tests/thermochimica/typed_outputs.i block=ChemicalComposition

Common parameters can be specified on the parent `[ChemicalComposition]` block and overridden in
named subblocks. See the [ChemicalComposition syntax](syntax/ChemicalComposition/index.md) for an
example using different databases on disjoint mesh blocks.

!syntax parameters /ChemicalComposition

!bibtex bibliography
