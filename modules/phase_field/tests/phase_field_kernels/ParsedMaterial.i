#
# Test the parsed function free enery Allen-Cahn Bulk kernel
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  zmin = 0
  zmax = 50
  elem_type = QUAD4
[]

[AuxVariables]
  [./eta]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 25.0
      y1 = 25.0
      radius = 15.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 20.0
    [../]
  [../]
[]

[Materials]
  [./consts]
    type = ParsedMaterial
    args  = 'eta'
    function ='(eta-0.5)^2'
    outputs = exodus
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
