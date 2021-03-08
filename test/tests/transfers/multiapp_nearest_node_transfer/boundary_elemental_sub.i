[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 6
  ny = 6
  nz = 6
[]

[Variables]
  [dummy]
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = dummy
  []
[]

[AuxVariables]
  [w]
  []
  [u_from_master_left_to_left]
    family = MONOMIAL
    order = CONSTANT
  []
  [u_from_master_top_to_top]
    family = MONOMIAL
    order = CONSTANT
  []
  [u_from_master_right_to_bottom]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [w]
    type = FunctionIC
    variable = w
    function = w
  []
[]

[Functions]
  [w]
    type = ParsedFunction
    value = 'x+2*y+z'
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
  hide = dummy
[]
