[Mesh]
  type = MFEMMesh
  file = ../../mesh/square_quad.e
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[FESpaces]
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = CONSTANT
  []
[]

[Variables]
  [mfem_scalar_var]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[AuxVariables]
  [libmesh_scalar_var]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[Functions]
  [parsed_function]
    type = ParsedFunction
    expression = 'x*x + y*y'
  []
[]

[ICs]
  [libmesh_scalar_var_ic]
    type = MFEMScalarIC
    variable = 'mfem_scalar_var'
    coefficient = parsed_function
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[MultiApps]
  [libmesh_app]
    type = FullSolveMultiApp
    input_files = libmesh_sub_scalar.i
    execute_on = 'INITIAL'
  []
[]

[Transfers]
  [transfer_from_libmesh]
    type = MultiApplibMeshToMFEMShapeEvaluationTransfer
    source_variable = libmesh_scalar_var
    variable = libmesh_scalar_var
    from_multi_app = libmesh_app
  []
[]

[Postprocessors]
  [Difference]
    type = MFEML2Error
    variable = mfem_scalar_var
    function = libmesh_scalar_var
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  file_base = 'mfem_parent_libmesh_sub_scalar_quads'
  csv = true
[]
