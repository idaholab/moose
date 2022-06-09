#
# KKS toy problem in the split form
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  nz = 0
  xmin = -2.5
  xmax = 2.5
  ymin = -2.5
  ymax = 2.5
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[AuxVariables]
  [./Fglobal]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Variables]
  # gas concentration
  [./cg]
    order = FIRST
    family = LAGRANGE
  [../]

  # vac concentration
  [./cv]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./cv]
    variable = cv
    type = SmoothCircleIC
    x1 = -0.5
    y1 = 0.0
    radius = 1.5
    invalue = 0.9
    outvalue = 0.1
    int_width = 0.75
  [../]
  [./cg]
    variable = cg
    type = SmoothCircleIC
    x1 = 0.5
    y1 = 0.0
    radius = 1.5
    invalue = 0.7
    outvalue = 0.0
    int_width = 0.75
  [../]
[]


[BCs]
  [./Periodic]
    [./all]
      variable = 'cg cv'
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  # Free energy of the matrix
  [./fm]
    type = KKSXeVacSolidMaterial
    property_name = fm
    cmg = cg
    cmv = cv
    T = 300
    outputs = exodus
    derivative_order = 2
  [../]
[]

[Kernels]
  [./diff_g]
    type = Diffusion
    variable = cg
  [../]
  [./time_g]
    type = TimeDerivative
    variable = cg
  [../]

  [./diff_v]
    type = Diffusion
    variable = cv
  [../]
  [./time_v]
    type = TimeDerivative
    variable = cv
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 3
  dt = 0.1
  petsc_options_iname = '-pctype -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = ' asm    lu          nonzero'
[]

[Outputs]
  file_base = kks_xevac
  exodus = true
[]
