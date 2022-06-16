[Mesh]
  type = GeneratedMesh
  dim = 1

  xmin = 0.0
  xmax = 10.0
  nx = 15
[]

[Variables]
  [./m]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./s_in]
    order = FIRST
    family = LAGRANGE
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
  [./s_in]
    type = CoupledForce
    variable = m
    v = s_in
  [../]
[]

[AuxKernels]
  [./reconstruct_s_in]
    type = FunctionSeriesToAux
    variable = s_in
    function = FX_Basis_Value_Main
  [../]
[]

[ICs]
  [./start_m]
    type = ConstantIC
    variable = m
    value = 1
  [../]
[]

[BCs]
  [./surround]
    type = DirichletBC
    variable = m
    value = 1
    boundary = 'left right'
  [../]
[]

[Functions]
  [./FX_Basis_Value_Main]
    type = FunctionSeries
    series_type = Cartesian
    orders = '3'
    physical_bounds = '0.0  10.0'
    x = Legendre
  [../]
[]

[UserObjects]
  [./FX_Value_UserObject_Main]
    type = FXVolumeUserObject
    function = FX_Basis_Value_Main
    variable = m
  [../]
[]

[Postprocessors]
  [./average_value]
    type = ElementAverageValue
    variable = m
  [../]
  [./peak_value]
    type = ElementExtremeValue
    value_type = max
    variable = m
  [../]
  [./picard_iterations]
    type = NumFixedPointIterations
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.5
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 30
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-9
  fixed_point_rel_tol = 1e-8
  fixed_point_abs_tol = 1e-9
[]

[MultiApps]
  [./FXTransferApp]
    type = TransientMultiApp
    input_files = multiapp_sub.i
  [../]
[]

[Transfers]
  [./ValueToSub]
    type = MultiAppFXTransfer
    to_multi_app = FXTransferApp
    this_app_object_name = FX_Value_UserObject_Main
    multi_app_object_name = FX_Basis_Value
  [../]
  [./ValueToMe]
    type = MultiAppFXTransfer
    from_multi_app = FXTransferApp
    this_app_object_name = FX_Basis_Value_Main
    multi_app_object_name = FX_Value_UserObject_Sub
  [../]
[]
