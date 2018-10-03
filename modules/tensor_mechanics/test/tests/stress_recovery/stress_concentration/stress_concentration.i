[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = FileMesh
  file = geo.msh
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xx_recovered]
  []
  [stress_yy_recovered]
  []
[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = 'timestep_end'
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = 'timestep_end'
  []
  [stress_xx_recovered]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx_recovered
    patch_polynomial_order = SECOND
    index_i = 0
    index_j = 0
    execute_on = 'timestep_end'
  []
  [stress_yy_recovered]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy_recovered
    patch_polynomial_order = SECOND
    index_i = 1
    index_j = 1
    execute_on = 'timestep_end'
  []
[]

[Kernels]
  [solid_x]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
  []
  [solid_y]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
  []
[]

[Materials]
  [strain]
    type = ComputeSmallStrain
  []
  [Cijkl]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 2.1e+5
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[BCs]
  [top_xdisp]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 'top'
    function = 0
  []
  [top_ydisp]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 'top'
    function = 0.01
  []
  [bottom_xdisp]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 'bottom'
    function = 0
  []
  [bottom_ydisp]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 'bottom'
    function = 0
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    ksp_norm = default
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-ksp_type -pc_type'
  petsc_options_value = 'preonly   lu'
  nl_rel_tol = 1e-14
  l_max_its = 100
  nl_max_its = 30
[]

[Outputs]
  interval = 1
  exodus = true
  print_linear_residuals = false
[]
