# Differences between MOOSE and the Geochemist's Workbench

The [Geochemists Workbench](https://www.gwb.com/) (GWB) software is a popular "gold-standard" geochemistry solver, and to help new users of the `geochemistry` module, many [tests and examples](geochemistry_tests_and_examples.md) are provided with GWB equivalents.  In addition, GWB is accompanyied by a textbook, [!cite](bethke_2007), which discusses these examples, and the `geochemistry` theory documentation uses notation and ideas drawn from this textbook in order to help users understand geochemistry concepts.  Users will undoubtably find the textbook valuable when exploring the `geochemistry` module's functionality.

On the other hand, the C++ code behind the `geochemistry` solver is different than GWB and [!cite](bethke_2007) because it is part of the MOOSE framework.  There are a number of differences that users should be aware of.  Most of these can be summarized by the following rule

!alert note
Rule: The geochemistry module respects the users constraints

## Setting bulk constraints

When the user sets `moles_bulk_species`, this means the mole number in the current basis (after the swaps).  For instance, the following means there is:

- 1 mole of Na$^{+}$.  There is also additional Na$^{+}$ inside the other basis species Mirabilite
- 1 mole of Cl$^{-}$
- 1 free mole of Mirabilite precipitate floating around in the solution, which contains 2 moles of Na$^{+}$, 1 mole of SO$_{4}^{2-}$ and 10 moles of H$_{2}$O

!listing modules/geochemistry/test/tests/equilibrium_models/bulk_constraints.i

The result is shown in [table:bulk_constraints].  Note that Mirabilite has a bulk mole number of 1.895.  One mol of this Mirabilite is the free quantity, and 0.895$\,$mol is dissolved into solution and has added to the molality of the other species (Na$^{+}$, NaSO$_{4}^{-}$, SO$_{4}^{2-}$ and NaCl).

A GWB input file that looks equivalent is

!listing modules/geochemistry/test/tests/equilibrium_models/bulk_constraints.rea

In fact, this does not converge in GWB (unless the charge-balance species is moved to Na$^{+}$).  The file that is actually equivalent sets

```
Na+ = 2.79 mol
```

The additional 1.79$\,$mol is twice the 0.895$\,$mol of dissolved Mirabilite (each mole of Mirabilite contains two moles of Na$^{+}$).  I'm not sure how to compute this number a-priori.


!table id=table:bulk_constraints caption=Species molality for the `bulk_constraints` simulation
| Species | Molality |
| --- | --- |
| Na$^{+}$ | 2.266 |
| Cl$^{-}$ | 0.976 |
| NaSO$_{4}^{-}$ | 0.4991 |
| SO$_{4}^{2-}$ | 0.3957 |
| NaCl | 0.02402 |
| Mirabilite | 1.895 *bulk* mol |


## Setting activity

When the user sets a constraint on a species' activity then MOOSE respects that constraint until it is turned off.  For instance, in the following input file the pH is set to 6 and remains so throughout the simulation, even as the Calcite is precipitating.

!listing modules/geochemistry/test/tests/equilibrium_models/ph_constraint.i

The final result is 97.92$\,$g of free calcite in the solution at pH 6.

This is different from the seemingly-equivalent GWB input file:

!listing modules/geochemistry/test/tests/equilibrium_models/ph_constraint.rea

The [Geochemists Workbench](https://www.gwb.com/) software:

1. Equilibrates the solution without precipitating minerals at pH 6
2. Closes the system so that the bulk mole composition of H$^{+}$ is fixed
3. Allows the Calcite precipitates to form, which alters the pH

The final result is 25.98$\,$g of free calcite precipitate at pH 4.689.

If a user wants to replicate the GWB procedure, two models must be run in succession: the first has no minerals and the user records the bulk composition of each species; the second has minerals and the bulk composition fixed.  The `geochemistry` module can be enhanced if this proves to be an annoying feature.

- A `close_before_precipitating` flag can be added to the [TimeIndependentReactionSolver](AddTimeIndependentReactionSolverAction.md).
- A `prevent_precipitation_on` input can be added to the [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md).  It is an `AuxVariable`: when it is 1.0 (the default) then precipitation of minerals specified in `prevent_precipitation` is prevented, otherwise precipitates can form (ie, the `prevent_precipitation` list is ignored).
- Both of these involve simple manipulations of the `_prevent_precipitation` data of the `GeochemicalSolver`.

## Setting bulk composition with kinetic species

When kinetic species are present in the problem, the initial bulk mole composition constraint must include the mole number present in the kinetic species.  This is different from GWB, in which the bulk mole number is refers to the species in the solution without the kinetic species.  A good example (that includes further discussion) is found on the [kinetic dissolution of albite](kinetic_albite.md) page.

## Temperature dependence

### Special temperature

Inspecting the [tests and examples](geochemistry_tests_and_examples.md) will reveal that the flag `piecewise_linear_interpolation` is frequently set true in the [GeochemicalModelDefinition](GeochemicalModelDefinition.md).  This is because that at the special temperatures 0, 25, 60, 100 150, 200, 250 and 300$^{\circ}$C, GWB does not perform a fourth-order least-squares fit to the equilibrium-constant data.  Instead, it uses the data exactly as tabulated in the database file.

### High temperatures

In all benchmarks run, the `geochemistry` module results agree exactly with GWB when the temperature is less than roughly 150$^{\circ}$C.  However, the results sometimes differ above this value.  The small discrepancies are probably due to slightly different interpolations of equilibrium constants, although it is difficult to be completely sure without having access to the GWB code.


## Stoichiometric ionic strength

The GWB software calculates the stoichiometric ionic strength (for computing the water activity via the Debye-Huckel model) using the Cl$^{-}$ molality only.  Hence, most of the [tests and examples](geochemistry_tests_and_examples.md) use the flag `stoichiometric_ionic_str_using_Cl_only = true`.











!bibtex bibliography