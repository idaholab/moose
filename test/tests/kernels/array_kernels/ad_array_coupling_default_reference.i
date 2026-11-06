# Reference for ad_array_coupling_default.i with the constant coupling term as a body force
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
    components = 2
  []
[]

[Kernels]
  [diff_u]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
  [source]
    type = ArrayBodyForce
    variable = u
    function = '2 1.5'
  []
[]

[BCs]
  [all]
    type = ArrayDirichletBC
    variable = u
    boundary = 'left right top bottom'
    values = '0 0'
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1 1'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Postprocessors]
  [u0]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 0
  []
  [u1]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 1
  []
[]

[Outputs]
  csv = true
  file_base = ad_array_coupling_default_out
[]
