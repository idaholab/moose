[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 5
    ny = 5
    extra_element_integers = 'material_id'
  []
  [set_material_id0]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0 0 0'
    top_right = '0.8 0.6 0'
    block_id = 0
    location = INSIDE
    integer_name = material_id
  []
  [set_material_id1]
    type = SubdomainBoundingBoxGenerator
    input = set_material_id0
    bottom_left = '0 0 0'
    top_right = '0.8 0.6 0'
    block_id = 1
    location = OUTSIDE
    integer_name = material_id
  []
[]

[Variables]
  [u]
    family = L2_LAGRANGE
    order = FIRST
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = dc
  []
  [timederivative]
    type = TimeDerivative
    variable = u
  []
  [sourceterm]
    type = BodyForce
    variable = u
    function = 1
  []
[]

[DGKernels]
  [dg_diff]
    type = DGDiffusion
    variable = u
    diff = dc
    epsilon = -1
    sigma = 6
  []
[]

[AuxVariables]
  [id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [id]
    type = ElementIntegerAux
    variable = id
    integer_names = material_id
  []
[]

[BCs]
  [vacuum]
    type = VacuumBC
    variable = u
    boundary = 'right left top bottom'
 []
[]

[Materials]
  [dc]
    type = ConstantIDMaterial
    prop_name = dc
    prop_values = '1 2'
    id_name = material_id
  []
[]

[Postprocessors]
  [unorm]
    type = ElementL2Norm
    variable = u
  []
[]

[Executioner]
  type = Transient

  end_time = 0.1
  dt = 0.01
  nl_abs_tol = 1.e-15
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
