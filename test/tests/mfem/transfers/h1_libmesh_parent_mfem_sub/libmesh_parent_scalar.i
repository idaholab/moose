[Mesh]
  file = ../../mesh/square_quad.e
[]

[Problem]
  type = FEProblem
[]

[Variables]
  [libmesh_scalar_var]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxVariables]
  [mfem_scalar_var] # libmesh representation of mfem variable
    family = LAGRANGE
    order = FIRST
  []
[]

[BCs]
  [sides]
    type = DirichletBC
    variable = libmesh_scalar_var
    boundary = 'bottom left right top'
    value = 1.0
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = libmesh_scalar_var
  []
  [source]
    type = BodyForce
    variable = libmesh_scalar_var
    value = 2.0
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = 'libmesh_parent_scalar_mfem_sub_quads'
  exodus = true
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
    source_variable = mfem_scalar_var
    variable = mfem_scalar_var
    from_multi_app = mfem_app
  []
[]
