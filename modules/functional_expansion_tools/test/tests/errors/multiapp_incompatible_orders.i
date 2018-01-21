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
    function = FE_Basis_Value_Main
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
  [./FE_Basis_Value_Main]
    type = FunctionSeries
    series_type = Cartesian
    orders = '36'
    physical_bounds = '0.0  10.0'
    x = Legendre
  [../]
[]

[UserObjects]
  [./FE_Value_UserObject_Main]
    type = FEVolumeUserObject
    function = FE_Basis_Value_Main
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
    type = NumPicardIterations
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.5
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  picard_max_its = 30
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-9
  picard_rel_tol = 1e-8
  picard_abs_tol = 1e-9
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./FETransferApp]
    type = TransientMultiApp
    input_files = multiapp_sub.i
  [../]
[]

[Transfers]
  [./ValueToSub]
    type = MultiAppFETransfer
    direction = to_multiapp
    multi_app = FETransferApp
    this_app_object_name = FE_Value_UserObject_Main
    multi_app_object_name = FE_Basis_Value_Sub
  [../]
  [./ValueToMe]
    type = MultiAppFETransfer
    direction = from_multiapp
    multi_app = FETransferApp
    this_app_object_name = FE_Basis_Value_Main
    multi_app_object_name = FE_Value_UserObject_Sub
  [../]
[]
