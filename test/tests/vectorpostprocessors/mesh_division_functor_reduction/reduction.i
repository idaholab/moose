[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 10
  # This ensures the CSV VPP output remains ordered the same way with distributed meshes
  allow_renumbering = false
[]

[MeshDivisions]
  [mesh_div]
    type = CylindricalGridDivision
    n_radial = 4
    n_azimuthal = 4
    axis_direction = '0 0 1'
    azimuthal_start = '1 0 0'
    center = '0.5 0.5 0'
    r_max = 2
  []
[]

[Functions]
  [u_fn]
    type = ParsedFunction
    expression = 'x + 2 * if(y > 0.5, y, 0)'
  []
[]

[AuxVariables]
  [u]
  []
  [u_fv]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [u_ic]
    type = FunctionIC
    variable = u
    function = u_fn
  []
  [u_fvic]
    type = FunctionIC
    variable = u_fv
    function = u_fn
  []
[]

[FunctorMaterials]
  [u_mat]
    type = ADGenericFunctorMaterial
    prop_names = 'u_mat'
    prop_values = 'u'
  []
[]

[VectorPostprocessors]
  [integral]
    type = MeshDivisionFunctorReductionVectorPostprocessor
    functors = 'u_fn u u_fv u_mat'
    mesh_division = mesh_div
    reduction = 'integral'
    execute_on = 'initial'
  []
  [average]
    type = MeshDivisionFunctorReductionVectorPostprocessor
    functors = 'u_fn u u_fv u_mat'
    mesh_division = mesh_div
    reduction = 'average'
    execute_on = 'initial'
  []
  [min]
    type = MeshDivisionFunctorReductionVectorPostprocessor
    functors = 'u_fn u u_fv u_mat'
    mesh_division = mesh_div
    reduction = 'min'
    execute_on = 'initial'
  []
  [max]
    type = MeshDivisionFunctorReductionVectorPostprocessor
    functors = 'u_fn'
    mesh_division = mesh_div
    reduction = 'max'
    execute_on = 'initial'
  []

  [sample_max]
    type = SpatialUserObjectVectorPostprocessor
    userobject = 'max'
    points = '0 0 0.1
              0.8 0 0'
    execute_on = 'initial'
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
  execute_on = 'initial'
[]
