#
# Test the parsed function free enery Allen-Cahn Bulk kernel
#

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 8
  ny = 8
  nz = 8
  xmax = 20
  ymax = 20
  zmax = 20
[]

[Variables]
  [./eta]
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0.0
      y1 = 0.0
      radius = 12.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 16.0
    [../]
  [../]
[]

[Kernels]
  [./detadt]
    type = TimeDerivative
    variable = eta
  [../]

  [./ACInterfaceStress]
    type = ACInterfaceStress
    variable = eta
    mob_name = 1
    stress = 2.7
  [../]
[]

[Materials]
  [./strain]
    type = GenericConstantRankTwoTensor
    tensor_name = elastic_strain
    tensor_values = '0.11 0.12 0.13 0.21 0.22 0.23 0.31 0.32 0.33'
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'
  solve_type = 'PJFNK'

  l_max_its = 15
  l_tol = 1.0e-4

  nl_max_its = 10
  nl_rel_tol = 1.0e-11

  start_time = 0.0
  num_steps = 2
  dt = 1000
[]

[Outputs]
  exodus = true
[]
