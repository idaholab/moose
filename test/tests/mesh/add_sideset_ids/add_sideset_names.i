[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  add_sideset_names = 'AdditionalSidesetA'
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [diff2]
    type = MatDiffusion
    diffusivity = 1e-4
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [flux]
    type = SideIntegralVariablePostprocessor
    variable = 'u'
    boundary = 'AdditionalSidesetA'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[UserObjects]
  [side_updater]
    type = SidesetAroundSubdomainUpdater
    inner_subdomains = 0
    outer_subdomains = 0
    update_sideset_name = 'AdditionalSidesetA'
    execute_on = TIMESTEP_END
    execution_order_group = -1
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
[]
