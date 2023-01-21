#
# Coupling volumetric material property inside boundary restricted material
# Also bringing boundary restricted material inside another boundary restricted
# material
#
# Solving: k \Laplace u + u - f = 0
#
# u = x^2 + y^2
# k = 3, but is decomposed as k3vol = k1vol + k2vol, where k1vol = 1 and k2vol = 2
#
# Boundary material property is computed as k3bnd = k1vol + k2bnd
#
# The material properties with suffix `vol` are volumetric, the ones with suffix `bnd`
# are boundary restricted
#

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
  [./exact_fn]
    type = ParsedFunction
    expression = x*x+y*y
  [../]

  [./f_fn]
    type = ParsedFunction
    expression = -4*3+x*x+y*y
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[Kernels]
  [./diff]
    type = DiffMKernel
    variable = u

    offset = 0
    mat_prop = k3vol
  [../]

  [./r]
    type = Reaction
    variable = u
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = f_fn
  [../]
[]

[BCs]
  [./all]
    type = MatDivergenceBC
    variable = u
    prop_name = k3bnd
    boundary = 'left right top bottom'
  [../]
[]

[Materials]
  [./k1vol]
    type = GenericConstantMaterial
    prop_names = 'k1vol'
    prop_values = 1
    block = 0
  [../]

  [./k2vol]
    type = GenericConstantMaterial
    prop_names = 'k2vol'
    prop_values = 2
    block = 0
  [../]
  [./k2bnd]
    type = GenericConstantMaterial
    prop_names = 'k2bnd'
    prop_values = 2
    boundary = 'left right top bottom'
  [../]

  [./k3vol]
    type = SumMaterial
    sum_prop_name = k3vol
    mp1 = k1vol
    mp2 = k2vol
    block = 0

    val1 = 1
    val2 = 2
  [../]

  [./k3bnd]
    type = SumMaterial
    sum_prop_name = 'k3bnd'
    mp1 = k1vol
    mp2 = k2bnd
    boundary = 'left right top bottom'

    val1 = 1
    val2 = 2
  [../]
[]

[Postprocessors]
  [./l2err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]
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
