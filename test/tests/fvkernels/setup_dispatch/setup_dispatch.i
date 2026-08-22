# Every finite volume family gets one do-nothing object that counts the setup methods it
# receives. The test asserts that initialSetup, timestepSetup and customSetup reach all of them,
# the expected counts being one initialSetup for the run and one timestepSetup and one customSetup
# per step. Three steps are taken so that a method dispatched twice per step shows up as six
# rather than having to be wrong at a single step.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1 0 0'
    block_id = 1
    top_right = '2 1 0'
  []
  [interface]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'interface'
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
    coeff = 1
  []
  [setup_elemental]
    type = FVSetupTestElementalKernel
    variable = u
  []
  [setup_flux]
    type = FVSetupTestFluxKernel
    variable = u
  []
[]

[FVBCs]
  # The two families cannot share a sideset, so the Dirichlet and flux test objects take one
  # boundary each. A fixed value on the left and zero flux on the right is well posed.
  [left]
    type = FVSetupTestDirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = FVSetupTestFluxBC
    variable = u
    boundary = right
  []
[]

[FVInterfaceKernels]
  [setup_ik]
    type = FVSetupTestInterfaceKernel
    variable1 = u
    boundary = interface
    subdomain1 = '0'
    subdomain2 = '1'
  []
[]

[Postprocessors]
  # One count per family per setup method. Counting rather than merely detecting a call is what
  # catches a method being dispatched more than once, which is what happens when a query is not
  # restricted to a single solver system. The custom counts are restricted to a single execution
  # flag because many flags route through customSetup, most of them unrelated to finite volume,
  # so an aggregate count would be an unreadable number that moves whenever those flags change.
  [elemental_initial]
    type = FVSetupCount
    object = setup_elemental
    count_type = INITIAL
  []
  [elemental_timestep]
    type = FVSetupCount
    object = setup_elemental
    count_type = TIMESTEP
  []
  [elemental_custom]
    type = FVSetupCount
    object = setup_elemental
    count_type = CUSTOM
    exec_flag = TIMESTEP_END
  []
  [flux_kernel_initial]
    type = FVSetupCount
    object = setup_flux
    count_type = INITIAL
  []
  [flux_kernel_timestep]
    type = FVSetupCount
    object = setup_flux
    count_type = TIMESTEP
  []
  [flux_kernel_custom]
    type = FVSetupCount
    object = setup_flux
    count_type = CUSTOM
    exec_flag = TIMESTEP_END
  []
  [dirichlet_bc_initial]
    type = FVSetupCount
    object = left
    count_type = INITIAL
  []
  [dirichlet_bc_timestep]
    type = FVSetupCount
    object = left
    count_type = TIMESTEP
  []
  [dirichlet_bc_custom]
    type = FVSetupCount
    object = left
    count_type = CUSTOM
    exec_flag = TIMESTEP_END
  []
  [flux_bc_initial]
    type = FVSetupCount
    object = right
    count_type = INITIAL
  []
  [flux_bc_timestep]
    type = FVSetupCount
    object = right
    count_type = TIMESTEP
  []
  [flux_bc_custom]
    type = FVSetupCount
    object = right
    count_type = CUSTOM
    exec_flag = TIMESTEP_END
  []
  [interface_kernel_initial]
    type = FVSetupCount
    object = setup_ik
    count_type = INITIAL
  []
  [interface_kernel_timestep]
    type = FVSetupCount
    object = setup_ik
    count_type = TIMESTEP
  []
  [interface_kernel_custom]
    type = FVSetupCount
    object = setup_ik
    count_type = CUSTOM
    exec_flag = TIMESTEP_END
  []
[]

[Outputs]
  csv = true
[]

[Executioner]
  type = Transient
  num_steps = 3
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]
