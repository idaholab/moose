[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  displacements = 'disp_x disp_y'
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [c]
  []
  [disp_x]
  []
  [disp_y]
  []
[]

[Kernels]
  [source]
    type = ADBodyForce
    variable = c
    function = source_func
    use_displaced_mesh = true
    displacements = ''
    #displacements = 'disp_x disp_y'
  []
  [dt]
    type = ADTimeDerivative
    variable = c
  []
[]

[Functions]
  [source_func]
    type = ParsedFunction
    expression = 'x + y^2'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 1
[]
