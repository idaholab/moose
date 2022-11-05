[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 25
  ny = 25
  nz = 0
  xmax = 250
  ymax = 250
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./c]
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 125.0
      y1 = 125.0
      radius = 60.0
      invalue = 1.0
      outvalue = 0.1
      int_width = 50.0
    [../]
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  [../]
  [./left]
    type = DirichletBC
    boundary = left
    variable = disp_x
    value = 0.0
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]
  [./dummy]
    type = MatDiffusion
    variable = c
    diffusivity = 0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '3 1 1 3 1 3 1 1 1 '
  [../]
  [./strain]
    type = ComputeSmallStrain
    eigenstrain_names = eigenstrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./prefactor]
    type = DerivativeParsedMaterial
    coupled_variables = c
    property_name = prefactor
    constant_names       = 'epsilon0 c0'
    constant_expressions = '0.05     0'
    expression = '(c - c0) * epsilon0'
  [../]
  [./eigenstrain]
    type = ComputeVariableEigenstrain
    eigen_base = '1'
    args = c
    prefactor = prefactor
    eigenstrain_name = eigenstrain
  [../]

  [./elasticenergy]
    type = ElasticEnergyMaterial
    args = 'c'
    outputs = exodus
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  nl_abs_tol = 1e-10
  num_steps = 1

  petsc_options_iname = '-pc_factor_shift_type'
  petsc_options_value = 'nonzero'
[]

[Outputs]
  exodus = true
[]
