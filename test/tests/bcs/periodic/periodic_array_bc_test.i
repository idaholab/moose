[Mesh]
  inactive = 'rotation'
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 40
    ymax = 40
    elem_type = QUAD4
  []
  [rotation]
    type = TransformGenerator
    input = gmg
    transform = "ROTATE"
    vector_value = '45 0 0'
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    components = 4
  []
[]

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = 'diff'
  []
  [force]
    type = ArrayBodyForce
    function = 'x y x*x (x*y+1)'
    variable = u
  []
  [dot]
    type = ArrayTimeDerivative
    variable = u
  []
[]

[BCs]
  [Periodic]
    [x]
      variable = u
      primary = 3
      secondary = 1
      translation = '40 0 0'
    []
  []
[]

[Materials]
  [diff]
    type = GenericConstantArray
    prop_name = 'diff'
    prop_value = '1 2 3 4'
    constant_on = SUBDOMAIN
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 2
  solve_type = NEWTON
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = array_out
  exodus = true
[]
