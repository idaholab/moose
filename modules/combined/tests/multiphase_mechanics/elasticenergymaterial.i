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
    disp_x = disp_x
    disp_y = disp_y
  [../]
  [./dummy]
    type = MatDiffusion
    variable = c
  [../]
[]

[Materials]
  [./eigenstrain]
    type = SimpleEigenStrainMaterial
    block = 0
    epsilon0 = 0.05
    c = c
    disp_y = disp_y
    disp_x = disp_x
    C_ijkl = '3 1 1 3 1 3 1 1 1 '
    fill_method = symmetric9
  [../]
  [./elasticenergy]
    type = ElasticEnergyMaterial
    block = 0
    args = 'c'
    outputs = exodus
  [../]
  [./genconst]
    type = GenericConstantMaterial
    block = 0
    prop_names  = 'D'
    prop_values = '0'
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
  output_initial = true
  exodus = true
  print_perf_log = true
[]
