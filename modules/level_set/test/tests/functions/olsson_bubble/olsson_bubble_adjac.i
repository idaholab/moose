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
  [bubble]
  []
  [disp_x]
  []
  [disp_y]
  []
[]

[Kernels]
  [bubble]
    type = ADBodyForce
    variable = bubble
    function = bubble_func
    use_displaced_mesh = true
  []
  [dt]
    type = ADTimeDerivative
    variable = bubble
  []
[]

[Functions]
  [bubble_func]
    type = LevelSetOlssonBubble
    center = '0.5 0.5 0'
    radius = 0.4
    epsilon = 0.05
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
