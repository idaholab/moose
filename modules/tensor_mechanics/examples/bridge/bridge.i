[GlobalParams]
  disp_x = disp_x
  disp_y = disp_y
  disp_z = disp_z
[]

[Mesh]
  displacements = 'disp_x disp_y disp_z'
  type = FileMesh
  file = bridge.e
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./gravity_y]
    type = GravityTM
    variable = disp_y
    value = -9.81
  [../]
  [./TensorMechanics]
  [../]
[]

[AuxVariables]
  [./von_mises]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./von_mises_kernel]
    type = RankTwoScalarAux
    variable = von_mises
    rank_two_tensor = stress
    execute_on = timestep_end
    scalar_type = VonMisesStress
  [../]
[]

[BCs]
  [./PressureTM]
    [./load]
      boundary = 1
      factor = 5e5 # Pa
    [../]
  [../]
  [./anchor_x]
    type = PresetBC
    variable = disp_x
    boundary = '2 3 4 5 6'
    value = 0.0
  [../]
  [./anchor_y]
    type = PresetBC
    variable = disp_y
    boundary = '2 3 4 5 6'
    value = 0.0
  [../]
  [./anchor_z]
    type = PresetBC
    variable = disp_z
    boundary = '2 3 4 5 6'
    value = 0.0
  [../]
[]

[Materials]
  active = 'density_steel stress strain elasticity_tensor_steel'
  [./elasticity_tensor_steel]
    youngs_modulus = 210e9 #Pa
    poissons_ratio = 0.3
    type = ComputeIsotropicElasticityTensor
    block = 1
  [../]
  [./elasticity_tensor_concrete]
    youngs_modulus = 16.5e9 #Pa
    poissons_ratio = 0.2
    type = ComputeIsotropicElasticityTensor
    block = 1
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 1
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = 1
  [../]
  [./density_steel]
    type = GenericConstantMaterial
    block = 1
    prop_names = density
    prop_values = 7850 # kg/m^3
  [../]
  [./density_concrete]
    type = GenericConstantMaterial
    block = 1
    prop_names = density
    prop_values = 2400 # kg/m^3
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-9
  l_max_its = 30
  l_tol = 1e-4
  nl_max_its = 10
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'
[]

[Outputs]
  print_linear_residuals = true
  print_perf_log = true
  output_initial = true
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]

