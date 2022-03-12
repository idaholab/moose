[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 3
    ny = 3
    dim = 2
  []
[]

[Variables]
  [u]
    order = FIRST
    family = MONOMIAL
    components = 1
    array = true
  []
[]

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
  [reaction]
    type = ArrayReaction
    variable = u
    reaction_coefficient = rc
  []
  [source]
    type = ArrayBodyForce
    variable = u
    function = '1'
  []
[]

[BCs]
  [all]
    type = ArrayVacuumBC
    boundary = 'left right top bottom'
    variable = u
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1'
  []
  [rc]
    type = GenericConstantArray
    prop_name = rc
    prop_value = '2'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  [out]
    type = Exodus
  []
[]
