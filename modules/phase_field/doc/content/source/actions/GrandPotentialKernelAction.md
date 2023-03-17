# GrandPotentialKernelAction

!syntax description /Modules/PhaseField/GrandPotential/GrandPotentialKernelAction


For more information on the multi-phase, multi-order parameter grand potential model, see [!cite](AagesenGP2018) (which is based on [!cite](plapp_unified_2011) and
[!cite](moelans_quantitative_2008)). Documentation on the multi-phase, multi-order parameter grand potential model itself, including MOOSE implementation, is available here:

[](/GrandPotentialMultiphase.md)

In this action, the following kernels are generated for each chemical potential variable:

- [`SusceptibilityTimeDerivative`](/SusceptibilityTimeDerivative.md)
- [`MatDiffusion`](/MatDiffusion.md) or [`MatAnisoDiffusion`](/MatAnisoDiffusion.md)
- [`CoupledSwitchingTimeDerivative`](/CoupledSwitchingTimeDerivative.md) (multiple kernels: one that corresponds with each order parameter)

The following kernels are generated for each order parameter:

- [`TimeDerivative`](/TimeDerivative.md)
- [`ACInterface`](/ACInterface.md)
- [`ACSwitching`](/ACSwitching.md)
- [`ACGrGrMulti`](/ACGrGrMulti.md)

Any additional kernels needed for your application will have to be entered manually into the input file.
There is a large number of inputs for this action, but they can be categorized into four
basic groups

### Global Inputs

These are applied throughout the kernels.

- `switching_function_names`: Vector of names of switching functions. These are used to distinguish between phases.
- `use_displacesd_mesh`: Standard option for kernels. This will be applied to all kernels generated.
- `implicit`: Standard option for kernels. This will be applied to all kernels generated.

### Chemical Potential Functions

These define the behavior of the chemical potential variables.

- `chemical_potentials`: Vector of names of chemical potential variables.
- `susceptibilities`: Vector of names of susceptibilities, chi. This vector should be the same length as "chemical_potentials" as each entry in this vector corresponds to the same entry in "chemical_potentials".
- `mobilities`: Vector of mobilities--either scalars or tensors--that correspond to the "chemical_potentials" variables. The entries should consist of diffusivities multiplied by susceptibilities.
- `anisotropic`: If the entries in "D" are tensors, set this to "true".
- `free_energies_w`: Vector of density functions that determine the densities corresponding with each "chemical_potentials". The total number of entries is the number of chemical potentials times the number of switching functions.

### Primary Set of Order Parameter Functions

Inputs affecting a group of auto-generated variables using the PolycrystalVarible
action. These typically represent grains in a polycrystal system.

- `op_num`: Number of order parameters auto-generated in Variable block.
- `var_name_base`: Name of order parameters auto-generated in Variable block.
- `free_energies_gr`: Vector of chemical potential density functions used for this set of order parameters. Each entry corresponds to the phase in the same entry of "switching_function_names".
- `mobility_name_gr`: Name of scalar mobility used with this set of order parameters.
- `energy_barrier_gr`: Name of energy barrier coefficient (m in [!cite](moelans_quantitative_2008).) used with this set of order parameters.
- `gamma_gr`: Name of gamma coefficient used with this set of order parameters which controls interface energy between these order parameters.
- `kappa_gr`: Name of kappa coefficient to be used with this set of order parameters.

### Second Set of Order Parameter Functions

This optional set can include additional order parameters to distinguish phases
or some other field not associated with the first set of order parameters.

- `additional_ops`: Vector of additional order parameters used in the model. Optional.
- `free_energies_op`: Vector of chemical potential density functions used for this set of order parameters. Each entry corresponds to the phase in the same entry of "switching_function_names".
- `mobility_name_op`: Name of scalar mobility used with this set of order parameters. If "additional_ops" is blank then this value and the others below will not be called and their values do not matter.
- `energy_barrier_op`: Name of energy barrier coefficient used with this set of order parameters.
- `gamma_op`: Name of gamma coefficient used with this set of order parameters which controls interface energy between these order parameters.
- `gamma_grxop`: Cross term gamma coefficient that controls the interface energy between the primary and second set of order parameters.
- `kappa_op`: Name of kappa coefficient to be used with this set of order parameters.

!syntax parameters /Modules/PhaseField/GrandPotential/GrandPotentialKernelAction

### Strict Mass Conservation formulation

This optional formulation is enforcing a strict mass conservation by generating kernels for concentration, 
and coupled it with chemical potential from the Grand Potential model.

- 'mass_conservation' : set this to "true" to choose strict mass conservation formulation
- 'concentrations' : Vector of concentration variables used in the model.
- 'h_c_min' : Vector of coefficients for the BodyForce kernel in concentration-chemical potential coupling which indicates the minima of the parabolic free energy functions
- 'h_over_kVa' : Vector of coefficients for the MatReaction kernel in concentration-chemical potential coupling which is related to the coefficients of the parabolic free enegy functions

The following kernels are generated for each concentration variables:

- [`TimeDerivative`](/TimeDerivative.md)
- [`MatDiffusion`](/MatDiffusion.md) or [`MatAnisoDiffusion`](/MatAnisoDiffusion.md)

The coupling between concentration and chemical potential is generated using the following kernel:

- [`MatReaction`](/MatReaction.md) 
- [`MatReaction`](/MatReaction.md) (multiple kernels: one that corresponds with each order parameter)
- ['BodyForce'](/BodyForce.md) (multiple kernels: one that corresponds with each order parameter)

The materials associated with strict mass conservation can be created automatically using GrandPotentialSinteringMaterial (/GrandPotentialSinteringMaterial.md)

## Example Input File Syntax

!listing modules/phase_field/test/tests/actions/gpm_kernel.i
    start=Modules
    end=Preconditioning

!bibtex bibliography
