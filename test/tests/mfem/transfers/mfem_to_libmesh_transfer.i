[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [shift_zero_block]
    type = RenameBlockGenerator
    old_block = '0'
    new_block = '1'
    input = gmg
  []
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
  [bottom]
    type = DirichletBC
    variable = libmesh_scalar_var
    boundary = 1
    value = 1.0
  []
  [top]
    type = DirichletBC
    variable = libmesh_scalar_var
    boundary = 3
    value = 0.0
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = libmesh_scalar_var
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
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
    input_files = mfem_diffusion.i
    execute_on = 'TIMESTEP_END'
  []
[]

[Transfers]
  [transfer_from_mfem]
    type = MultiAppMFEMlibMeshGeneralFieldTransfer
    source_variable = concentration
    variable = mfem_scalar_var
    from_multi_app = mfem_app
  []
[]