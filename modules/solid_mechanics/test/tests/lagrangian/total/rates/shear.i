# shear modulus
G = 5000

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
  []
  [sdz]
    type = TotalLagrangianStressDivergence
    variable = disp_z
    component = 2
  []
[]

[AuxVariables]
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
    [AuxKernel]
      type = RankTwoAux
      rank_two_tensor = cauchy_stress
      index_i = 0
      index_j = 0
      execute_on = 'INITIAL TIMESTEP_END'
    []
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
    [AuxKernel]
      type = RankTwoAux
      rank_two_tensor = cauchy_stress
      index_i = 1
      index_j = 1
      execute_on = 'INITIAL TIMESTEP_END'
    []
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
    [AuxKernel]
      type = RankTwoAux
      rank_two_tensor = cauchy_stress
      index_i = 0
      index_j = 1
      execute_on = 'INITIAL TIMESTEP_END'
    []
  []
[]

[BCs]
  [x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'top bottom' # This contains all 8 nodes in the patch
    function = 't*y'
  []
  [y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'top bottom' # This contains all 8 nodes in the patch
    function = '0'
  []
  [z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'top bottom' # This contains all 8 nodes in the patch
    function = '0'
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    lambda = ${G}
    shear_modulus = ${G}
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [compute_strain]
    type = ComputeLagrangianStrain
  []
[]

[Postprocessors]
  [sxx]
    type = ElementAverageValue
    variable = stress_xx
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = none
  []
  [sxx0]
    type = ParsedPostprocessor
    pp_names = 'sxx'
    function = 'sxx/${G}'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [syy]
    type = ElementAverageValue
    variable = stress_yy
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = none
  []
  [syy0]
    type = ParsedPostprocessor
    pp_names = 'syy'
    function = 'syy/${G}'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [sxy]
    type = ElementAverageValue
    variable = stress_xy
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = none
  []
  [sxy0]
    type = ParsedPostprocessor
    pp_names = 'sxy'
    function = 'sxy/${G}'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  dt = 0.05

  solve_type = NEWTON

  petsc_options_iname = -pc_type
  petsc_options_value = lu

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10

  end_time = 20
[]

[Outputs]
  csv = true
[]
