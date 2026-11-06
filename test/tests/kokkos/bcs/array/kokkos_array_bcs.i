# Solves -div(D grad u) = 0 on the unit square for a two-component array variable with u = 0 on
# the left boundary and D du/dn = h on the right boundary, where h = (1, 2). The top and bottom
# boundaries have zero flux, so the exact solution u = D^-1 h x is linear and is represented
# exactly by first-order elements, giving component integrals of D^-1 h / 2:
#   scalar D = 2:             (0.25, 0.5)
#   diagonal D = diag(2, 4):  (0.25, 0.25)
#   full D = [2 1; 1 3]:      (0.1, 0.3)

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    components = 2
  []
[]

[Kernels]
  [diff]
    type = KokkosArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
[]

[BCs]
  [left]
    type = KokkosArrayDirichletBC
    variable = u
    boundary = left
    values = '0 0'
  []
  [right]
    type = KokkosArrayNeumannBC
    variable = u
    boundary = right
    value = '1 2'
  []
[]

[Materials]
  active = 'scalar'
  [scalar]
    type = KokkosGenericConstantMaterial
    prop_names = dc
    prop_values = 2
  []
  [diagonal]
    type = KokkosConstantArrayDiffusivity
    prop_name = dc
    components = 2
    values = '2 4'
  []
  [full]
    type = KokkosConstantArrayDiffusivity
    prop_name = dc
    components = 2
    matrix = true
    # Row-major entries of [2 1; 1 3]
    values = '2 1 1 3'
  []
[]

[Postprocessors]
  [intu0]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 0
  []
  [intu1]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
