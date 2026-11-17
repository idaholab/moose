# AD values and gradients of an uncoupled array variable fall back to constant default values with
# zero gradients. With v = (2, 3) and coefficients (1, 0.5), the coupling kernel adds the constant
# source (2, 1.5) to both components of u, as the reference body force in
# ad_array_coupling_default_reference.i does.
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
    type = ADArrayDiffusion
    variable = u
  []
  [v_on_u]
    type = ADArrayCoupledValueGradient
    variable = u
    v = '2 3'
    coef = '1 0.5'
    direction = '1 2 0'
  []
[]

[BCs]
  [all]
    type = ADArrayDirichletBC
    variable = u
    boundary = 'left right top bottom'
    values = '0 0'
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
