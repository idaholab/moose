[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 5
  xmax = 10
  ymax = 10
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  displacement_gradients = 'gxx gxy gyx gyy'
[]

[Modules]
  [./PhaseField]
    [./DisplacementGradients]
    [../]
  [../]
[]

[AuxVariables]
  [./disp_x]
    [./InitialCondition]
      type = FunctionIC
      function = '0.1*sin(2*x/10*3.14159265359)'
    [../]
  [../]
  [./disp_y]
    [./InitialCondition]
      type = FunctionIC
      function = '0.1*sin(1*y/10*3.14159265359)'
    [../]
  [../]
[]

[Variables]
  [./c]
    order = THIRD
    family = HERMITE
    initial_condition = 0
  [../]
[]

[Kernels]
  [./dt]
    type = TimeDerivative
    variable = c
  [../]
  [./bulk]
    type = CahnHilliard
    variable = c
    mob_name = M
    f_name = F
  [../]
  [./int]
    type = CHInterface
    variable = c
    mob_name = M
    kappa_name = kappa_c
  [../]
[]

[BCs]
  [./Periodic]
    [./All]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'M  kappa_c'
    prop_values = '1  0.1'
  [../]

  [./elasticity_tensor]
    type = ComputeConcentrationDependentElasticityTensor
    c = c
    C0_ijkl = '1.0 1.0'
    C1_ijkl = '3.0 3.0'
    fill_method0 = symmetric_isotropic
    fill_method1 = symmetric_isotropic
  [../]
  [./smallstrain]
    type = ComputeSmallStrain
  [../]
  [./linearelastic_a]
    type = ComputeLinearElasticStress
  [../]
  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = F
    args = 'c'
    derivative_order = 3
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = NEWTON

  l_max_its = 30
  l_tol = 1.0e-6

  nl_max_its = 15
  nl_rel_tol = 1.0e-7
  nl_abs_tol = 1.0e-10

  num_steps = 2
  dt = 1
[]

[Outputs]
  perf_graph = true
  file_base = nonsplit_gradderiv_out
  exodus = true
[]
