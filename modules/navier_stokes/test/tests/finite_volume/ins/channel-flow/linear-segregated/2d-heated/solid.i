k = 2

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.25 0.25'
    dy = '0.2'
    ix = '5 5'
    iy = '5'
    subdomain_id = '0 1'
  []
  [delete]
    type = BlockDeletionGenerator
    input = mesh
    block = '0'
  []
[]

[Variables]
  [T_solid]
    type = MooseVariableFVReal
    initial_condition = 300
  []
[]

[AuxVariables]
  [T_fluid]
    type = MooseVariableFVReal
    initial_condition = 300
  []
[]

[MultiApps]
  inactive = 'fluid'
  [fluid]
    type = FullSolveMultiApp
    input_files = fluid.i
    execute_on = timestep_begin
    no_restore = true
  []
[]

[Transfers]
  inactive = 'from_fluid to_fluid'
  [from_fluid]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = fluid
    source_variable = 'T_fluid'
    variable = 'T_fluid'
    execute_on = timestep_begin
    from_blocks = 1
  []
  [to_fluid]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = fluid
    source_variable = 'T_solid'
    variable = 'T_solid'
    execute_on = timestep_begin
    to_blocks = 1
  []
[]

[FVKernels]
  [conduction]
    type = FVDiffusion
    variable = T_solid
    coeff = ${k}
  []
  [source]
    type = FVBodyForce
    variable = T_solid
    function = 25000
  []
  [heat_exchange]
    type = PINSFVEnergyAmbientConvection
    variable = T_solid
    h_solid_fluid = 100
    T_fluid = T_fluid
    T_solid = T_solid
    is_solid = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
