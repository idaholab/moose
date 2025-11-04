power = 2
E0 = 1.0
Emin = 1.0e-6

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  # final_generator = 'MoveRight'
  [Bottom]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 80
    ny = 40
    xmin = 0
    xmax = 150
    ymin = 0
    ymax = 75
  []
  [left_load]
    type = ExtraNodesetGenerator
    input = Bottom
    new_boundary = left_load
    coord = '37.5 75 0'
  []
  [right_load]
    type = ExtraNodesetGenerator
    input = left_load
    new_boundary = right_load
    coord = '112.5 75 0'
  []
  [left_support]
    type = ExtraNodesetGenerator
    input = right_load
    new_boundary = left_support
    coord = '0 0 0'
  []
  [right_support]
    type = ExtraNodesetGenerator
    input = left_support
    new_boundary = right_support
    coord = '150 0 0'
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [Dc]
    family = MONOMIAL
    order = SECOND
    initial_condition = -1.0
  []
  [Cc]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = -1.0
  []
  [mat_den]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0.1
  []
  [sensitivity_var]
    family = MONOMIAL
    order = SECOND
    initial_condition = -1.0
  []
[]

[AuxKernels]
  [sensitivity_kernel]
    type = MaterialRealAux
    property = sensitivity
    variable = sensitivity_var
    check_boundary_restricted = false
    execute_on = 'TIMESTEP_END'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    add_variables = true
    incremental = false
  []
[]

[BCs]
  [no_y]
    type = DirichletBC
    variable = disp_y
    boundary = left_support
    value = 0.0
  []
  [no_x]
    type = DirichletBC
    variable = disp_x
    boundary = left_support
    value = 0.0
  []
  [no_y_right]
    type = DirichletBC
    variable = disp_y
    boundary = right_support
    value = 0.0
  []
[]

[NodalKernels]
  [push_left]
    type = NodalGravity
    variable = disp_y
    boundary = left_load
    gravity_value = -1e-3
    mass = 1
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeVariableIsotropicElasticityTensor
    youngs_modulus = E_phys
    poissons_ratio = poissons_ratio
    args = 'mat_den'
  []
  [E_phys]
    type = DerivativeParsedMaterial
    # Emin + (density^penal) * (E0 - Emin)
    expression = '${Emin} + (mat_den ^ ${power}) * (${E0}-${Emin})'
    coupled_variables = 'mat_den'
    property_name = E_phys
  []
  [poissons_ratio]
    type = GenericConstantMaterial
    prop_names = poissons_ratio
    prop_values = 0.0
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [dc]
    type = ComplianceSensitivity
    design_density = mat_den
    youngs_modulus = E_phys
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[UserObjects]
  [rad_avg]
    type = RadialAverage
    radius = 3
    weights = linear
    prop_name = sensitivity
    force_preaux = true
    execute_on = 'TIMESTEP_END'
  []
  # No SIMP optimization in subapp
  [calc_sense]
    type = SensitivityFilter
    density_sensitivity = Dc
    design_density = mat_den
    filter_UO = rad_avg
    force_postaux = true
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  nl_abs_tol = 1e-10
  dt = 1.0
  num_steps = 25
[]

[Outputs]
  exodus = true
  [out]
    type = CSV
    execute_on = 'TIMESTEP_END'
  []
  print_linear_residuals = false
[]

[Postprocessors]
  [mesh_volume]
    type = VolumePostprocessor
    execute_on = 'initial timestep_end'
  []
  [total_vol]
    type = ElementIntegralVariablePostprocessor
    variable = mat_den
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [vol_frac]
    type = ParsedPostprocessor
    expression = 'total_vol / mesh_volume'
    pp_names = 'total_vol mesh_volume'
  []
  [sensitivity]
    type = ElementIntegralMaterialProperty
    mat_prop = sensitivity
    execute_on = 'TIMESTEP_BEGIN TIMESTEP_END NONLINEAR'
  []
  [objective]
    type = ElementIntegralMaterialProperty
    mat_prop = strain_energy_density
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
