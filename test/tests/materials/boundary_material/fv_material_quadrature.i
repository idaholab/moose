# Parsed material properties depend on the physical location of the element
# This requires the initialization of the quadrature in the FVFlux loop

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 4
  ny = 4
  elem_type = QUAD9
[]

[Functions]
  [linear_x]
    type = ParsedFunction
    expression = 'x'
  []
  [piecewise_linear_x]
    type = PiecewiseLinear
    x = '-1 2'
    y = '-1 2'
    axis = 'x'
  []
[]

[Variables]
  [u]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
    coeff = k1
    coeff_interp_method = average
  []

  [r]
    type = FVReaction
    variable = u
  []
[]

[FVBCs]
  [all]
    type = FVDirichletBC
    variable = u
    boundary = 'left right bottom top'
    value = 1
  []
[]

[Materials]
  active = 'k1'
  [k1]
    type = ADGenericFunctorMaterial
    prop_names = 'k1'
    prop_values = linear_x
    block = 0
  []
  [k1_piecewise]
    type = ADGenericFunctorMaterial
    prop_names = 'k1'
    prop_values = piecewise_linear_x
    block = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
