[GlobalParams]
  displacements = 'x_disp y_disp z_disp'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 3
  ny = 3
  nz = 3
  elem_type = HEX
[]

[Functions]
  [./rampConstant]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = 1e-6
  [../]
[]

[Variables]
  [./x_disp]
    order = FIRST
    family = LAGRANGE
  [../]
  [./y_disp]
    order = FIRST
    family = LAGRANGE
  [../]
  [./z_disp]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
 [../]
 [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
 [../]
[]

[AuxKernels]
  [./vonmises]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = vonmises
    scalar_type = VonMisesStress
  [../]
[]

[VectorPostprocessors]
  [./vonmises]
    type = LineMaterialRankTwoScalarSampler
    start = '0.1667 0.4 0.45'
    end   = '0.8333 0.6 0.55'
    property = stress
    scalar_type = VonMisesStress
    sort_by = id
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
  [../]
[]

[BCs]
  [./front]
    type = FunctionDirichletBC
    variable = z_disp
    boundary = 5
    function = rampConstant
  [../]
  [./back_x]
    type = DirichletBC
    variable = x_disp
    boundary = 0
    value = 0.0
  [../]
  [./back_y]
    type = DirichletBC
    variable = y_disp
    boundary = 0
    value = 0.0
  [../]
  [./back_z]
    type = DirichletBC
    variable = z_disp
    boundary = 0
    value = 0.0
  [../]
[]

[Materials]
  [./elast_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = .3
  [../]
  [./strain]
    type = ComputeSmallStrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK

  l_max_its = 100

  start_time = 0.0
  num_steps = 99999
  end_time = 1.0
  dt = 0.1
[]

[Outputs]
  file_base = rank_two_scalar_sampler_out
  csv = true
[]
