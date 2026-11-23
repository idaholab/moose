[Mesh]
  [mycart]
    type = CartesianMeshGenerator
    dim = 2
    dx = '3 3'
    dy = '4 4'
    subdomain_id = '1 2
                    3 4'
  []

  # A sideset interior to the preserved subdomain, which only survives because the elements
  # carrying it are copied rather than dualized
  [inner]
    type = ParsedGenerateSideset
    input = mycart
    combinatorial_geometry = 'y > 3.9 & y < 4.1'
    included_subdomains = '3'
    new_sideset_name = 'inner_b3'
  []

  [my_id]
    type = ParsedExtraElementIDGenerator
    input = inner
    expression = 'if(x < 3, 10, 20)'
    extra_elem_integer_name = 'my_id'
  []

  [myDualGen]
    type = DualMeshGenerator
    input = my_id
    dual_mesh_type = barycentric
    preserve_primal_subdomains = '3'
  []

  [myDiagnosticGenerator]
    type = MeshDiagnosticsGenerator
    input = myDualGen
    examine_element_overlap = ERROR
    examine_element_volumes = ERROR
    minimum_element_volumes = 0
    examine_non_conformality = ERROR
  []
[]

[AuxVariables]
  [my_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [unity]
  []
[]

[AuxKernels]
  [my_id]
    type = ExtraElementIDAux
    variable = my_id
    extra_id_name = 'my_id'
  []
  [unity]
    type = ConstantAux
    variable = unity
    value = 1
  []
[]

[Postprocessors]
  # The preserved primal elements are the only ones with my_id set, so this reports the value they
  # carried over from the input mesh
  [preserved_my_id]
    type = ElementExtremeValue
    variable = my_id
    value_type = max
  []

  # Counts the nodes of the sideset interior to the preserved subdomain, which the dual mesh keeps
  # only because those elements are copied rather than dualized. The side integral postprocessors
  # cannot be used here, as they need a face normal that polygon sides do not provide.
  [inner_b3_nodes]
    type = NodalSum
    variable = unity
    boundary = 'inner_b3'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]

