# A mortar constraint reads a gap conductance produced by a material that reads a property from
# another material. Both producers must be computed on the secondary and primary interior parents.
[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    nx = 2
    ny = 2
    boundary_name_prefix = lb
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block
    subdomain_id = 1
  []
  [right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 2
    xmax = 3
    nx = 2
    ny = 2
    boundary_name_prefix = rb
    boundary_id_offset = 10
  []
  [right_block_id]
    type = SubdomainIDGenerator
    input = right_block
    subdomain_id = 2
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'left_block_id right_block_id'
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    input = combined
    sidesets = 'lb_right'
    new_block_id = 10001
    new_block_name = secondary_lower
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    input = secondary
    sidesets = 'rb_left'
    new_block_id = 10000
    new_block_name = primary_lower
  []
[]

[Variables]
  [T]
    block = '1 2'
  []
  [lambda]
    block = secondary_lower
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [conduction]
    type = Diffusion
    variable = T
    block = '1 2'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = T
    boundary = lb_left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = T
    boundary = rb_right
    value = 0
  []
[]

[Functions]
  [zero]
    type = ConstantFunction
    value = 0
  []
[]

[Constraints]
  [mortar]
    type = GapHeatConductanceTest
    primary_boundary = rb_left
    secondary_boundary = lb_right
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    secondary_variable = T
    variable = lambda
    secondary_gap_conductance = gap_conductance
    primary_gap_conductance = gap_conductance
    secondary_mms_function = zero
    primary_mms_function = zero
  []
[]

[Materials]
  # gap_conductance = 1 + A = 2, only consumed by the mortar constraint, and A only through
  # gap_conductance
  [A]
    type = ADMaterialReinitTest
    block = '1 2'
    property = A
    value = 1
  []
  [gap_conductance]
    type = ADMaterialReinitTest
    block = '1 2'
    property = gap_conductance
    value = 1
    coupled_property = A
  []
[]

[Postprocessors]
  # Conductivity 1 in each unit-width block and gap conductance h over a gap of width 1, in series:
  # T(1) = (1 + 1/h) / (2 + 1/h), which is 3/5 for h = 2 (and 2/3 for h = 1)
  [T_secondary]
    type = PointValue
    variable = T
    point = '1 0.5 0'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
[]
