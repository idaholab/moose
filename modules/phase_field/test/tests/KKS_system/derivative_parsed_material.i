#
# This test validates the free energy material with automatic differentiation for the KKS system
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  nz = 0
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = c1
    boundary = 'left'
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = c1
    boundary = 'right'
    value = 1
  [../]
  [./top]
    type = DirichletBC
    variable = c2
    boundary = 'top'
    value = 0
  [../]
  [./bottom]
    type = DirichletBC
    variable = c2
    boundary = 'bottom'
    value = 1
  [../]
[]

[Variables]
  # concentration 1
  [./c1]
    order = FIRST
    family = LAGRANGE
  [../]

  # concentration 2
  [./c2]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Materials]
  [./fa]
    type = DerivativeParsedMaterial
    property_name = F
    coupled_variables = 'c1 c2'
    constant_names       = 'T    kB'
    constant_expressions = '400  .000086173324'
    expression = 'c1^2+100*T*kB*(c2-0.5)^3+c1^4*c2^5'
    outputs = exodus
  [../]
[]

[Kernels]
  [./c1diff]
    type = Diffusion
    variable = c1
  [../]

  [./c2diff]
    type = Diffusion
    variable = c2
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = derivative_parsed_material
  exodus = true
[]
