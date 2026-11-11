# Generic bidirectional reduced-porosity globalization regression. Prescribed uniform compression
# drives pore collapse with no gas or EOS state. A deliberately under-iterated and damped coupled
# solve must search downward in porosity and recover a mechanically equilibrated FREE root.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmax = 1e-3
  ymax = 1e-3
[]

[Physics/SolidMechanics/QuasiStatic/All]
  strain = FINITE
  add_variables = true
  use_automatic_differentiation = true
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [bottom]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [right]
    type = ADDirichletBC
    variable = disp_x
    boundary = right
    value = -8e-7
  []
  [top]
    type = ADDirichletBC
    variable = disp_y
    boundary = top
    value = -8e-7
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.3
  []
  [creep_coefficient]
    type = ADGenericConstantMaterial
    prop_names = creep_coefficient
    prop_values = 1e-9
  []
  [stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = lps
  []
  [porosity]
    type = ADPorosityFromStrain
    initial_porosity = 0.05
    inelastic_strain = combined_inelastic_strain
  []
  [lps]
    type = ADPorousViscoplasticityStressUpdate
    coefficient = creep_coefficient
    power = 1
    initial_porosity = 0.05
    minimum_porosity = 1e-10
    max_inelastic_increment = 1e-2
    local_newton_tolerance = 1e-6
    local_newton_stagnation_tolerance = 1e-5
    local_newton_max_iterations = 1
    local_newton_relaxation = 0.5
    reduced_porosity_root_max_iterations = 64
    verbose = true
  []
[]

[Postprocessors]
  [porosity]
    type = ADElementAverageMaterialProperty
    mat_prop = porosity
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[UserObjects]
  [check_porosity_collapse]
    type = Terminator
    expression = 'porosity != porosity | porosity >= 0.0499999999 | porosity <= 0'
    execute_on = FINAL
    fail_mode = HARD
    error_level = ERROR
    message = 'Generic reduced-porosity downward recovery did not produce finite pore collapse.'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.01
  num_steps = 1
  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-10
  l_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = false
  print_linear_residuals = false
[]
