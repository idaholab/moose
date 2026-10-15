# ITER Cylindrical Geometries - Base Configuration
#
# This file contains common physics, functions, and solver settings for
# copper_cylinder.i, cylinder.i, and annulus.i
#
# To use: Define mesh in your geometry-specific file, then !include this file
#
# Cannot run standalone - requires:
#   - Mesh with pin_bottom_0_deg, pin_bottom_90_deg, pin_bottom_180_deg nodesets
#   - Material overrides defining final youngs_modulus and poissons_ratio properties
#   - Thermal expansion material defining eigenstrain
#   - Outputs block with file_base specification

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Physics/SolidMechanics]
  [QuasiStatic]
    [all]
      strain = FINITE
      add_variables = true
      eigenstrain_names = 'thermal_expansion'
      generate_output = 'vonmises_stress stress_xx stress_zz stress_xy'
      temperature = T
      use_automatic_differentiation = true
    []
  []
[]

[BCs]
  # Constrain rigid body modes: 3 translations + 3 rotations
  [pin_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = 'pin_bottom_90_deg'
    value = 0
  []
  [pin_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = 'pin_bottom_0_deg pin_bottom_180_deg'
    value = 0
  []
  [pin_z]
    type = ADDirichletBC
    variable = disp_z
    boundary = 'pin_bottom_0_deg pin_bottom_90_deg pin_bottom_180_deg'
    value = 0
  []
[]

[Kernels]
  [lorentz_x]
    type = ADBodyForce
    variable = disp_x
    function = lorentz_x
    use_displaced_mesh = true
  []
  [lorentz_y]
    type = ADBodyForce
    variable = disp_y
    function = lorentz_y
    use_displaced_mesh = true
  []
  [lorentz_z]
    type = ADBodyForce
    variable = disp_z
    function = lorentz_z
    use_displaced_mesh = true
  []
[]

[Materials]
  # Base component properties - defined globally, no block restriction
  # Variants select which to use via their material overrides

  # Copper (OFHC) properties
  [youngs_modulus_Cu]
    type = ADParsedMaterial
    property_name = youngs_modulus_cu
    coupled_variables = T
    expression = '1e3*(137-1.27e-04*T^2)' # Source: Page 6-1, NIST Monograph 177
  []

  [poissons_ratio_Cu]
    type = ADParsedMaterial
    property_name = poissons_ratio_cu
    coupled_variables = T
    expression = '0.339 + 7.03e-08*T^2' # Source: Page 6-23, NIST Monograph 177
  []

  # Nb3Sn properties - placeholders
  # Defined in base but only used by cylinder.i and annulus.i
  [youngs_modulus_nb3sn]
    type = ADGenericConstantMaterial
    prop_names = 'youngs_modulus_nb3sn'
    prop_values = '100e3' # Placeholder: 100 GPa in MPa
  []

  [poissons_ratio_nb3sn]
    type = ADGenericConstantMaterial
    prop_names = 'poissons_ratio_nb3sn'
    prop_values = '0.3' # Placeholder
  []

  # Mechanics materials - use 'youngs_modulus' and 'poissons_ratio'
  # which variants define via their material overrides
  [elasticity]
    type = ADComputeVariableIsotropicElasticityTensor
    youngs_modulus = youngs_modulus
    poissons_ratio = poissons_ratio
  []

  [stress]
    type = ADComputeFiniteStrainElasticStress
  []
[]

[AuxVariables]
  [T]
    family = LAGRANGE
    order = FIRST
    initial_condition = '${starting_temperature}'
  []
  [lorentz_x_aux]
    family = LAGRANGE
    order = FIRST
  []
  [lorentz_y_aux]
    family = LAGRANGE
    order = FIRST
  []
  [lorentz_z_aux]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
  [temperature_ramp]
    type = FunctionAux
    variable = T
    function = 'temperature_func'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [lorentz_x_aux_kernel]
    type = FunctionAux
    variable = lorentz_x_aux
    function = lorentz_x
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [lorentz_y_aux_kernel]
    type = FunctionAux
    variable = lorentz_y_aux
    function = lorentz_y
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [lorentz_z_aux_kernel]
    type = FunctionAux
    variable = lorentz_z_aux
    function = lorentz_z
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Functions]
  [temperature_func]
    type = PiecewiseLinear
    x = '0 ${simulation_time}'
    y = '${starting_temperature} ${ending_temperature}'
  []

  # NIST OFHC Copper thermal expansion coefficient correlation (4-300 K)
  [thermal_expansion_func_cu]
    type = ParsedFunction
    # 't' parameter represents temperature when used by thermal expansion material
    expression = '1e-6 * (10^(c0 + c1*log10(t) + c2*log10(t)^2 + c3*log10(t)^3 + c4*log10(t)^4 + c5*log10(t)^5 + c6*log10(t)^6))'
    symbol_names = 'c0 c1 c2 c3 c4 c5 c6'
    symbol_values = '-17.9081289 67.131914 -118.809316 109.9845997 -53.8696089 13.30247491 -1.30843441'
  []

  # Nb3Sn thermal expansion (placeholder: constant)
  # Defined in base but only used by cylinder.i and annulus.i
  [thermal_expansion_func_nb3sn]
    type = ParsedFunction
    expression = '5e-6' # Placeholder: constant 5e-6 / K
  []

  # Effective thermal expansion: 2/3 Cu + 1/3 Nb3Sn
  # Defined in base but only used by cylinder.i and annulus.i
  [thermal_expansion_func_effective]
    type = LinearCombinationFunction
    functions = 'thermal_expansion_func_cu thermal_expansion_func_nb3sn'
    w = '${fparse 2.0/3.0} ${fparse 1.0/3.0}'
  []

  ### Electromagnetic field functions for Lorentz force: f = J x B ###

  # Axial Current density: J = [0, 0, J_z]
  [jx]
    type = ParsedFunction
    expression = 0
  []
  [jy]
    type = ParsedFunction
    expression = 0
  []
  [jz]
    type = ParsedFunction
    expression = '${current_density_z}'
  []

  # Azimuthal magnetic field: B = (mu_0 J_z / 2)[-y, x, 0]
  [bx]
    type = ParsedFunction
    expression = '-${vacuum_permeability}*jz*y/2'
    symbol_names = 'jz'
    symbol_values = 'jz'
  []
  [by]
    type = ParsedFunction
    expression = '${vacuum_permeability}*jz*x/2'
    symbol_names = 'jz'
    symbol_values = 'jz'
  []
  [bz]
    type = ParsedFunction
    expression = '0'
  []

  # Radial Lorentz force: f = J x B
  [lorentz_x]
    type = ParsedFunction
    expression = 'jy*bz - jz*by'
    symbol_names = 'jy bz jz by'
    symbol_values = 'jy bz jz by'
  []
  [lorentz_y]
    type = ParsedFunction
    expression = 'jz*bx - jx*bz'
    symbol_names = 'jz bx jx bz'
    symbol_values = 'jz bx jx bz'
  []
  [lorentz_z]
    type = ParsedFunction
    expression = 'jx*by - jy*bx'
    symbol_names = 'jx by jy bx'
    symbol_values = 'jx by jy bx'
  []
[]

[Postprocessors]
  # Simulation Constants
  [stress_free_temperature]
    type = ConstantPostprocessor
    value = ${stress_free_temperature}
    execute_on = 'INITIAL'
    outputs = 'csv'
  []
  [starting_temperature]
    type = ConstantPostprocessor
    value = ${starting_temperature}
    execute_on = 'INITIAL'
    outputs = 'csv'
  []
  [ending_temperature]
    type = ConstantPostprocessor
    value = ${ending_temperature}
    execute_on = 'INITIAL'
    outputs = 'csv'
  []
  [simulation_time]
    type = ConstantPostprocessor
    value = ${simulation_time}
    execute_on = 'INITIAL'
    outputs = 'csv'
  []
  [current_density_z]
    type = ConstantPostprocessor
    value = ${current_density_z}
    execute_on = 'INITIAL'
    outputs = 'csv'
  []
  [vacuum_permeability]
    type = ConstantPostprocessor
    value = ${vacuum_permeability}
    execute_on = 'INITIAL'
    outputs = 'csv'
  []

  # Global averages
  [temperature_average]
    type = ElementAverageValue
    variable = T
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [stress_xx_pp]
    type = ElementAverageValue
    variable = stress_xx
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'csv'
  []
  [stress_zz_pp]
    type = ElementAverageValue
    variable = stress_zz
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'csv'
  []
  [stress_xy_pp]
    type = ElementAverageValue
    variable = stress_xy
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'csv'
  []
  [vonmises_stress_pp]
    type = ElementAverageValue
    variable = vonmises_stress
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'csv'
  []
[]


[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'preonly   lu       mumps'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-16
  end_time = ${simulation_time}
  num_steps = 10
  line_search = 'basic'
[]
