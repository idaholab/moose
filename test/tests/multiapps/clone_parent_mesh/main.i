[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables/u]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [src]
    type = BodyForce
    variable = u
  []
[]

[BCs]
  [dirichlet]
    type = DirichletBC
    variable = u
    boundary = 'top right'
    value = 0
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[AuxVariables]
  [u_sub]
  []
  [diff]
  []
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = sub.i
    clone_parent_mesh = true
  []
[]

[Transfers]
  [transfer]
    type = MultiAppCopyTransfer
    from_multi_app = sub
    variable = u_sub
    source_variable = u
  []
[]

[AuxKernels]
  [diff_aux]
    type = ParsedAux
    variable = diff
    expression = 'abs(u - u_sub)'
    coupled_variables = 'u u_sub'
  []
[]

[Postprocessors]
  [diff_max]
    type = ElementExtremeValue
    variable = diff
  []
[]

[UserObjects]
  [terminator]
    type = Terminator
    expression = 'diff_max > 1e-8'
    fail_mode = HARD
    error_level = ERROR
  []
[]

[Outputs]
  exodus = true
[]
