# The `geochemistry` database reader

A description of the GWB database format has been given [elsewhere](db_description.md).  This page describes the database reader of the `geochemistry` module.

## Database format

The database contains info on the following

- The "standard" temperatures \[$^{\circ}$C\] at which the equilibrium coefficients, activity coefficients, etc, are provided.
- The type of interpolation used to compute equilibrium coefficients, etc, at other temperatures other than standard temperatures.
- The steam saturation curve: pressures \[bar = $10^{5}\,$Pa\] at the standard temperatures.
- The [Debye-Huckel coefficients](activity_coefficients.md) evaluated at the standard temperatures
- The neutral-species [activity coefficients](activity_coefficients.md) evaluated at the standard temperatures
- The coefficients required to compute the [activity of water](activity_coefficients.md) evaluated at the standard temperatures
- Definitions of the names, chemical formulae and mole weight \[g\] of elements.
- The basis species, including their

  - name
  - charge
  - ion size (for activity computation)
  - mole weight \[g\]
  - elemental decomposition

- Optionally, information regarding sorbing sites (which are simply considered as another basis species by the code), including their

  - name
  - charge
  - mole weight \[g\]
  - elemental decomposition

- Optionally, the redox pairs, including their

  - name
  - charge
  - ion size (for activity computation)
  - mole weight \[g\]
  - equilibrium reaction stoichiometry, which may involve all basis species as well as redox pairs that have already been defined (the file is read in order)
  - the equilibrium constant, evaluated at each of the standard temperatures, for the equilibrium reaction

- Optionally, secondary aqueous species, including their

  - name
  - charge
  - ion size (for activity computation)
  - mole weight \[g\]
  - equilibrium reaction stoichiometry, which may involve all basis species as well as redox pairs
  - the equilibrium constant, evaluated at each of the standard temperatures, for the equilibrium reaction

- Optionally, information concerning the free electron, including its

  - name
  - charge
  - ion size (for activity computation)
  - mole weight \[g\]
  - equilibrium reaction stoichiometry, which may involve all basis species as well as redox pairs
  - the equilibrium constant, evaluated at each of the standard temperatures, for the equilibrium reaction

- Optionally, information concerning minerals, including their

  - name
  - mole volume \[cm$^{3}$\]
  - mole weight \[g\]
  - equilibrium reaction stoichiometry, which may involve all basis species as well as redox pairs
  - the equilibrium constant, evaluated at each of the standard temperatures, for the equilibrium reaction

- Optionally, information concerning gases, including their

  - name
  - coefficients for computing fugacity
  - equilibrium reaction stoichiometry, which may involve all basis species as well as redox pairs
  - the equilibrium constant, evaluated at each of the standard temperatures, for the equilibrium reaction

- Optionally, information regarding sorbing minerals that are involved in surface complexation reactions, including their

  - name, which must be one of the minerals already defined
  - their specific surface area \[m$^{2}$.g$^{-1}$\]
  - information regarding the density \[mol/mol(mineral)\] of each sorbing sites on their surface.  The sorbing sites must have already been defined, as above

- Optionally, information concerning surface species involved in sorbing, including their

  - name
  - charge
  - mole weight \[g\]
  - equilibrium reaction stoichiometry, which may involve all basis species, redox pairs and will involve at least one sorbing site (which are considered as just another basis species by the code)
  - $\log_{10}K$ for the reaction, as well as its derivative with respect to temperature


## Reader workflow

### Initial parsing

The user specifies the filename and the `reader` parses the file checking for errors, recording the standard temperatures, interpolation type, the steam-saturation curve information and the coefficients required for computing activity.  The following lists are built.

- A list of the elements
- A list of all basis components, which includes:

  - the species called "basis species" in the database, and
  - all sorbing sites (if any) and
  - all redox pairs (if any).
  
- A list of all secondary species, including the free electron.
- A list of all minerals.
- A list of all gases.
- A list of sorbing minerals.
- A list of surface species involved in sorbing.

### Eliminating unused species

Not all the species listed in the database are used in most geochemical models, which allows much of the information in the database to be eliminated.  General information concerning the basis is written [here](basis.md).

#### Information required to enable elimination

To eliminate useless information, the Reader expects the user to supply the following information.

1. A list of basis components relevant to their problem.  These components must be chosen from the "basis species" in the database, the sorbing sites and the decoupled redox states that are in disequilibrium (if any).  In the Geochemists Workbench [!citep](bethke_2007), the basis is specified implicitly through the user providing [swap](swap.md) information, initial pH, initial free concentration, initial bulk concentration, a basis component involved in charge balance, etc.  However, for unambigious clarity, `geochemistry` demands that the user specify the basis components, before specifying swaps, initial conditions, etc.

2. A list of species whose dynamics are governed by [kinetic rate laws](theory/index.md).  These can include:

- redox pairs that are decoupled (so are in disequilibrium);
- minerals that slowly precipitate or dissolve;
- surface species that slowly sorb.

3. A list of minerals to ignore throughout the entire computation.  This can be "ignore all minerals".  Since the reader will eliminate any knowledge of these minerals, they will never precipitate, can never be given an initial condition, will never appear in the basis after swapping, and will never be involved in surface complexation.

4. A list of minerals to include in the computation, so they have the possibility to appear at least some spatial location at some time.  This takes precidence over the list of minerals to ignore.  For instance `ignore_mineral = all` and `include_mineral = Fe(OH)3(ppd)` means that Fe(OH)$_{3}$(ppd) will be included, but no other minerals will be included.

5. A list of gases whose fugacity will be fixed, at least for some time at some spatial location.

#### Eliminating equilibrium redox species

All redox species that are in equilibrium with the aqueous solution may be eliminated from the database.  From the list of all redox components, the following are removed:

- any redox components specified in the user's list of basis components (these are in disequilibrium), and
- any redox species that are governed by kinetic rate laws.

The remaining redox species are in equilibrium.  They may be eliminated by using their reactions that are specified in the database.  This must be performed from "top to bottom" in the database file, since redox species can occur in other redox species' reactions.

#### Eliminating unimportant species

Having eliminated all equilibrium redox species, the Reader can eliminate species that are unimportant in the simulation.  It retains species only if:

1. they are in the list of basis components specified by the user, or
2. their dynamics is governed by kinetic rates, or
3. they are minerals that have been explicitly included, or
4. they are secondary species whose equilibrium reaction includes only species contained in (1), (2) and (3), or
5. they are minerals whose equilibrium reaction includes includes only species contained in (1), (2) and (3), but aren't in the `ignore_mineral` list, or
6. they are gases whose equilibrium reaction includes only species contained in (1), (2) and (3) and are in the `include_gas` list, or
7. they are sorbing minerals whose sorbing sites includes only species in (1), but aren't in the `ignore_mineral` list, or
8. they are surface species whose reaction involves only species in (1).

After elimination, the Reader contains only information that is pertinent to the model.


## The Reader's interface

- Must support [swaps](swap.md), but shouldn't itself keep a list of the current basis, as this basis will potentially vary from node-to-node.  Instead, it should assume the basis is as specified by the user, and when a swap is needed, it should provide the new equilibrium constants, stoichiometries, etc

- Should do interpolation of temperature-dependent things

- Should be indexed using integers instead of the species names, which would be inefficient.

- Allow checking that initial conditions have been specified correctly.  This is related to the swaps.





