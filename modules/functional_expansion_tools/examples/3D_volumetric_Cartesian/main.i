# Basic example coupling a master and sub app in a 3D Cartesian volume.
#
# The master app provides field values to the sub app via Functional Expansions, which then performs
# its calculations. The sub app's solution field values are then transferred back to the master app
# and coupled into the solution of the master app solution.
#
# This example couples Functional Expansions via AuxVariable.
#
# Note: this problem is not light, and may take a few minutes to solve.
[Mesh]
  type = GeneratedMesh
  dim = 3

  xmin = 0.0
  xmax = 10.0
  nx = 15

  ymin = 1.0
  ymax = 11.0
  ny = 25

  zmin = 2.0
  zmax = 12.0
  nz = 35
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
    type = HeatConduction
    variable = m
  [../]
  [./time_diff_m]
    type = HeatConductionTimeDerivative
    variable = m
  [../]
  [./s_in] # Add in the contribution from the SubApp
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

[Materials]
  [./Unobtanium]
    type = GenericConstantMaterial
    prop_names =  'thermal_conductivity specific_heat density'
    prop_values = '1.0                  1.0           1.0' # W/(cm K), J/(g K), g/cm^3
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
    boundary = 'top bottom left right front back'
  [../]
[]

[Functions]
  [./FX_Basis_Value_Main]
    type = FunctionSeries
    series_type = Cartesian
    orders = '3   4   5'
    physical_bounds = '0.0  10.0    1.0 11.0    2.0 12.0'
    x = Legendre
    y = Legendre
    z = Legendre
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

[Outputs]
  exodus = true
[]

[MultiApps]
  [./FXTransferApp]
    type = TransientMultiApp
    input_files = sub.i
  [../]
[]

[Transfers]
  [./ValueToSub]
    type = MultiAppFXTransfer
    to_multi_app = FXTransferApp
    this_app_object_name = FX_Value_UserObject_Main
    multi_app_object_name = FX_Basis_Value_Sub
  [../]
  [./ValueToMe]
    type = MultiAppFXTransfer
    from_multi_app = FXTransferApp
    this_app_object_name = FX_Basis_Value_Main
    multi_app_object_name = FX_Value_UserObject_Sub
  [../]
[]
