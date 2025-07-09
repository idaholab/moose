[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0.0
  xmax = 0.4
  nx = 6
  ymin = 0.0
  ymax = 10.0
  ny = 20
[]

[Variables]
  [./m]
  [../]
[]

[Kernels]
  [./diff_m]
    type = Diffusion
    variable = m
  [../]
  [./time_diff_m]
    type = TimeDerivative
    variable = m
  [../]
  [./source_m]
    type = BodyForce
    variable = m
    value = 100
  [../]
[]

[ICs]
  [./start_m]
    type = ConstantIC
    value = 2
    variable = m
  [../]
[]

[BCs]
  [./interface_value]
    type = FXValueBC
    variable = m
    boundary = right
    function = FX_Basis_Value_Main
  [../]
  [./interface_flux]
    type = FXFluxBC
    boundary = right
    variable = m
    function = FX_Basis_Flux_Main
  [../]
[]

[Functions]
  [./FX_Basis_Value_Main]
    type = FunctionSeries
    series_type = Cartesian
    orders = '4'
    physical_bounds = '0.0 10'
    y = Legendre
  [../]
  [./FX_Basis_Flux_Main]
    type = FunctionSeries
    series_type = Cartesian
    orders = '5'
    physical_bounds = '0.0 10'
    y = Legendre
  [../]
[]

[UserObjects]
  [./FX_Flux_UserObject_Main]
    type = FXBoundaryFluxUserObject
    function = FX_Basis_Flux_Main
    variable = m
    boundary = right
    diffusivity = 0.1
  [../]
[]

[Postprocessors]
  [./average_interface_value]
    type = SideAverageValue
    variable = m
    boundary = right
  [../]
  [./total_flux]
    type = SideDiffusiveFluxIntegral
    variable = m
    boundary = right
    diffusivity = 0.1
  [../]
  [./picard_iterations]
    type = NumFixedPointIterations
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 4
  dt = 1.0
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 30
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-9
  fixed_point_rel_tol = 1e-8
  fixed_point_abs_tol = 1e-9
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./FXTransferApp]
    type = TransientMultiApp
    input_files = interface_sub.i
    sub_cycling = true
  [../]
[]

[Transfers]
  [./FluxToSub]
    type = MultiAppFXTransfer
    to_multi_app = FXTransferApp
    this_app_object_name = FX_Flux_UserObject_Main
    multi_app_object_name = FX_Basis_Flux_Sub
  [../]
  [./ValueToMe]
    type = MultiAppFXTransfer
    from_multi_app = FXTransferApp
    this_app_object_name = FX_Basis_Value_Main
    multi_app_object_name = FX_Value_UserObject_Sub
  [../]
  [./FluxToMe]
    type = MultiAppFXTransfer
    from_multi_app = FXTransferApp
    this_app_object_name = FX_Basis_Flux_Main
    multi_app_object_name = FX_Flux_UserObject_Sub
  [../]
[]
