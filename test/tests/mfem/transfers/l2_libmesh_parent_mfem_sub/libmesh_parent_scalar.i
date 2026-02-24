[Mesh]
  file = ../../mesh/square.e
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Variables]
  [libmesh_scalar_var]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxVariables]
  [mfem_scalar_var] # libmesh representation of mfem variable
    family = MONOMIAL
    order = CONSTANT
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
    type = FunctionIC
    variable = 'libmesh_scalar_var'
    function = parsed_function
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = 'libmesh_parent_scalar_mfem_sub_quads'
  csv = true
[]

[Postprocessors]
  [Difference]
    type = ElementL2Difference
    variable = libmesh_scalar_var
    other_variable = mfem_scalar_var
  []
[]

[MultiApps]
  [mfem_app]
    type = FullSolveMultiApp
    input_files = mfem_sub_scalar.i
    execute_on = 'INITIAL'
  []
[]

[Transfers]
  [transfer_from_mfem]
    type = MultiAppMFEMTolibMeshShapeEvaluationTransfer
    source_variable = temperature
    variable = mfem_scalar_var
    from_multi_app = mfem_app
  []
[]
