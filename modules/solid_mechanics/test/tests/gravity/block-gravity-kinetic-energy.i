starting_point = 2e-1
offset = 1.0

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = long-bottom-block-1elem-blocks.e
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Variables]
  [disp_x]
    block = '1 2'
  []
  [disp_y]
    block = '1 2'
  []
[]

[AuxVariables]
  [pid]
    order = CONSTANT
    family = MONOMIAL
  []
  [kinetic_energy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [pid]
    type = ProcessorIDAux
    variable = pid
    execute_on = 'initial timestep_end'
  []
  [kinetic_energy]
    type = KineticEnergyAux
    block = '1 2'
    variable = kinetic_energy
    newmark_velocity_x = vel_x
    newmark_velocity_y = vel_y
    newmark_velocity_z = 0.0
    density = density
  []
[]

[ICs]
  [disp_y]
    type = ConstantIC
    block = 2
    variable = disp_y
    value = '${fparse starting_point + offset}'
  []
[]

[Modules/TensorMechanics/DynamicMaster]
  [all]
    add_variables = true
    hht_alpha = 0.0
    beta = 0.25
    gamma = 0.5
    mass_damping_coefficient = 0.0
    stiffness_damping_coefficient = 0.0
    displacements = 'disp_x disp_y'
    generate_output = 'stress_xx stress_yy'
    block = '1 2'
    strain = FINITE
  []
[]

[Kernels]
  [gravity]
    type = Gravity
    value = -9.81
    variable = disp_y
  []
[]

[Materials]
  [elasticity_2]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e4
    poissons_ratio = 0.3
  []
  [elasticity_1]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e7
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  []
  [density]
    type = GenericConstantMaterial
    block = '1 2'
    prop_names = 'density'
    prop_values = '7750'
  []
[]

[BCs]
  [botx]
    type = DirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
  []
  [boty]
    type = DirichletBC
    variable = disp_y
    boundary = 40
    value = 0.0
  []
[]

[Executioner]
  type = Transient
  end_time = 0.5
  dt = 0.01
  dtmin = .05
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -pc_svd_monitor '
                  '-snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -mat_mffd_err '
                        '-ksp_gmres_restart'
  petsc_options_value = 'lu       NONZERO               1e-15                   1e-5          100'
  l_max_its = 100
  nl_max_its = 20
  line_search = 'none'
  snesmf_reuse_base = false
  [TimeIntegrator]
    type = NewmarkBeta
    beta = 0.25
    gamma = 0.5
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = false
  csv = true
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  active = 'total_kinetic_energy'
  [total_kinetic_energy]
    type = ElementIntegralVariablePostprocessor
    variable = kinetic_energy
    block = '1 2'
  []
[]
