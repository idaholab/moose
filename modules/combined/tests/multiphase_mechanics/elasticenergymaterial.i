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
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./c]
    order = FIRST
    family = LAGRANGE
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
    type = PresetBC
    boundary = bottom
    variable = disp_y
    value = 0.
  [../]
  [./left]
    type = PresetBC
    boundary = left
    variable = disp_x
    value = 0.
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]
  [./dummy]
    type = MatDiffusion
    variable = c
    D_name = 0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric9
    C_ijkl = '3 1 1 3 1 3 1 1 1 '
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 0
    displacements = 'disp_x disp_y'
    eigenstrain_names = eigenstrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = 0
  [../]
  [./prefactor]
    type = DerivativeParsedMaterial
    block = 0
    args = c
    f_name = prefactor
    constant_names       = 'epsilon0 c0'
    constant_expressions = '0.05     0'
    function = '(c - c0) * epsilon0'
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
    block = 0
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
