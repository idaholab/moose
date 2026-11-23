# SaltFoss MOOSE training examples

Thermochimica-coupled MOOSE examples built against the typed-output
`ChemicalComposition` interface (refs #22711).

| Input | Demonstrates |
| :-- | :-- |
| `01_interface_anatomy.i` | The published flat output selectors (`output_phases`, `output_species`, `output_element_potentials`, `output_vapor_pressures`, `output_element_phases`) and their typed `[Outputs]` equivalents, on the Mo-Ru noble-metal binary. |
| `02_redox_sweep.i` | A spatial FLiBe-UF4 redox sweep (reducing to oxidizing fluorine mass balance), using `ConstituentFractions` to read the U(III)/U(IV) MQMQA cation split directly. |
| `03_offgas.i` | Off-gassing at a pressure below the salt's bubble point, with `pressure` set on a named `ChemicalComposition` subblock. |
| `04_performance.i` | `warm_start` and `batch_size` on a 41-node spatial sweep, with `report_performance` counters. |
| `05_coupled_bc.i` | A Thermochimica element-potential output coupled directly into a finite-element boundary condition. |

## Required data file not included

`02_redox_sweep.i`, `03_offgas.i`, `04_performance.i`, and
`05_coupled_bc.i` need `MSD-TC_V4.1_Fluorides_No_Func.dat`, an
MSTDB-TC fluoride thermodynamic database. MSTDB-TC is export-
controlled / registration-gated and is not distributed in this
repository. Obtain it from the
[MSTDB project](https://mstdb.ornl.gov) and place it in this directory
(or edit `thermodynamic_database` in each input to point at your own
copy) before running those four examples. `01_interface_anatomy.i`
needs only the already-tracked `Kaye_NobleMetals.dat` and runs with no
extra setup.
