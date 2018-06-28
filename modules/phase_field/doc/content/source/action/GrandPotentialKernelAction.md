# GrandPotentialKernelAction

## Description

The GrandPotentialKernelAction automatically generates kernels needed for the Grand Potential Model.
For more information on the grand potential model, see [cite:plapp_unified_2011] and
[cite:moelans_quantitative_2008].
The following kernels are generated for each chemical potential variable:

- SusceptibilityTimeDerivative
- MatDiffusion or MatAnisoDiffusion
- CoupledSwitchingTimeDerivative (multiple kernels: one that corresponds with each order parameter)

The following kernels are generated for each order parameter:

- TimeDerivative
- ACInterface
- ACSwitching
- ACGrGrMulti

Any additional kernels needed for your application will have to be entered manually into the input file.
There is a large number of inputs for this action, but they can be categorized into four
basic groups:

- Global inputs: these are applied throughout the kernels.
- Chemical potential inputs: these define the behavior of the chemical potential variables.
- Primary set of order parameters: Inputs affecting a goup of auto-generated variables using the
  PolycrystalVarible action.
  These typically represent grains in a polycrystal system.
- Secondary set of order paramters: this optional set can include additional order parameters
  to distinguish phases or some other field not associated with the first set of order parameters.

All of the inputs in these groups are explained below:

### Global Inputs:

- switching_function_names: Vector of names of switching functions. These are used to distinguish between phases.
- use_displacesd_mesh: Standard option for kernels. This will be applied to all kernels generated.
- implicit: Standard option for kernels. This will be applied to all kernels generated.

### Chemical Potential Functions

- chemical_potentials: Vector of names of chemical potential variables.
- susceptibilities: Vector of names of susceptibilities, chi. This vector should be the same length as "chemical_potentials" as each entry in this vector corresponds to the same entry in "chemical_potentials".
- mobilities: Vector of mobilities--either scalars or tensors--that correspond to the "chemical_potentials" variables. The entries should consist of diffusivities multiplied by susceptibilities.
- anisotropic: If the entries in "D" are tensors, set this to "true".
- free_energies_w: Vector of density functions that determine the densities corresponding with each "chemical_potentials". The total number of entries is the number of chemical potentials times the number of switching functions.

### Primary Set of Order Parameter Functions

- op_num: Number of order parameters auto-generated in Variable block.
- var_name_base: Name of order parameters auto-generated in Variable block.
- free_energies_gr: Vector of chemical potential density functions used for this set of order parameters. Each entry corresponds to the phase in the same entry of "switching_function_names".
- mobility_name_gr: Name of scalar mobility used with this set of order parameters.
- energy_barrier_gr: Name of energy barrier coefficient (m in [cite:moelans_quantitative_2008].) used with this set of order parameters.
- gamma_gr: Name of gamma coefficient used with this set of order parameters which controls interface energy between these order parameters.
- kappa_gr: Name of kappa coefficient to be used with this set of order parameters.

### Second Set of Order Parameter Functions

- additional_ops: Vector of additional order parameters used in the model. Optional.
- free_energies_op: Vector of chemical potential density functions used for this set of order parameters. Each entry corresponds to the phase in the same entry of "switching_function_names".
- mobility_name_op: Name of scalar mobility used with this set of order parameters. If "additional_ops" is blank then this value and the others below will not be called and their values do not matter.
- energy_barrier_op: Name of energy barrier coefficient used with this set of order parameters.
- gamma_op: Name of gamma coefficient used with this set of order parameters which controls interface energy between these order parameters.
- gamma_grxop: Cross term gamma coefficient that controls the interface energy between the primary and second set of order parameters.
- kappa_op: Name of kappa coefficient to be used with this set of order parameters.

## Example Input File Syntax

```
[Modules]
  [./PhaseField]
    [./GrandPotential]
      switching_function_names = 'hb hm'
      anisotropic = false

      chemical_potentials = 'w'
      mobilities = 'chiD'
      susceptibilities = 'chi'
      free_energies_w = 'rhob rhom'

      gamma_gr = gamma
      mobility_name_gr = L
      kappa_gr = kappa
      free_energies_gr = 'omegab omegam'
      energy_barrier_gr = mu

      additional_ops = 'phi'
      gamma_grxop = gamma
      mobility_name_op = L_phi
      kappa_op = kappa
      free_energies_op = 'omegab omegam'
      energy_barrier_op = mu
    [../]
  [../]
[]

[Materials]
  #REFERENCES
  [./constants]
    type = GenericConstantMaterial
    prop_names =  'Va      cb_eq cm_eq kb   km  mu  gamma L      L_phi  kappa  kB'
    prop_values = '0.04092 1.0   1e-5  1400 140 1.5 1.5   5.3e+3 2.3e+4 295.85 8.6173324e-5'
  [../]
  #SWITCHING FUNCTIONS
  [./switchb]
    type = SwitchingFunctionMultiPhaseMaterial
    h_name = hb
    all_etas = 'phi eta0'
    phase_etas = 'phi'
  [../]
  [./switchm]
    type = SwitchingFunctionMultiPhaseMaterial
    h_name = hm
    all_etas = 'phi eta0'
    phase_etas = 'eta0'
  [../]
  [./omegab]
    type = DerivativeParsedMaterial
    f_name = omegab
    args = 'w phi'
    material_property_names = 'Va kb cb_eq'
    function = '-0.5*w^2/Va^2/kb - w/Va*cb_eq'
    derivative_order = 2
  [../]
  [./omegam]
    type = DerivativeParsedMaterial
    f_name = omegam
    args = 'w eta0'
    material_property_names = 'Va km cm_eq'
    function = '-0.5*w^2/Va^2/km - w/Va*cm_eq'
    derivative_order = 2
  [../]
  [./chi]
    type = DerivativeParsedMaterial
    f_name = chi
    args = 'w'
    material_property_names = 'Va hb hm kb km'
    function = '(hm/km + hb/kb)/Va^2'
    derivative_order = 2
  [../]
  #DENSITIES/CONCENTRATION
  [./rhob]
    type = DerivativeParsedMaterial
    f_name = rhob
    args = 'w'
    material_property_names = 'Va kb cb_eq'
    function = 'w/Va^2/kb + cb_eq/Va'
    derivative_order = 1
  [../]
  [./rhom]
    type = DerivativeParsedMaterial
    f_name = rhom
    args = 'w eta0'
    material_property_names = 'Va km cm_eq(eta0)'
    function = 'w/Va^2/km + cm_eq/Va'
    derivative_order = 1
  [../]
  [./concentration]
    type = ParsedMaterial
    f_name = c
    material_property_names = 'rhom hm rhob hb Va'
    function = 'Va*(hm*rhom + hb*rhob)'
    outputs = exodus
  [../]
  [./mobility]
    type = DerivativeParsedMaterial
    material_property_names = 'chi kB'
    constant_names = 'T Em D0'
    constant_expressions = '1400 2.4 1.25e2'
    f_name = chiD
    function = 'chi*D0*exp(-Em/kB/T)'
  [../]
[]
```
