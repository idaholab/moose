# Simple 1D plane strain test

[GlobalParams]
  displacements = 'disp_x'
  large_kinematics = true
  stabilize_strain = true
[]

[Variables]
  [disp_x]
  []
[]

[ICs]
  [disp_x]
    type = RandomIC
    variable = disp_x
    min = -0.1
    max = 0.1
  []
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
[]

[Functions]
  [pull]
    type = ParsedFunction
    expression = '200 * t'
  []
[]

[BCs]
  [leftx]
    type = DirichletBC
    preset = true
    boundary = right
    variable = disp_x
    value = 0.0
  []
  [pull]
    type = FunctionNeumannBC
    boundary = left
    variable = disp_x
    function = pull
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 100000.0
    poissons_ratio = 0.3
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [compute_strain]
    type = ComputeLagrangianStrain
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 5.0
  dtmin = 5.0
  end_time = 5.0
[]
