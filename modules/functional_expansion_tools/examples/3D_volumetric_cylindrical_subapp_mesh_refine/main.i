# Derived from the example '3D_volumetric_cylindrical' with the following differences:
#
#   1) The model mesh is refined in the MasterApp by 1
#   2) Mesh adaptivity is enabled for the SubApp
#   3) Output from the SubApp is enabled so that the mesh changes can be visualized
[Mesh]
  type = FileMesh
  file = cyl-tet.e
  uniform_refine = 1
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
    boundary = 'top bottom outside'
  [../]
[]

[Functions]
  [./FX_Basis_Value_Main]
    type = FunctionSeries
    series_type = CylindricalDuo
    orders = '5   3' # Axial first, then (r, t) FX
    physical_bounds = '-2.5 2.5   0 0 1' # z_min z_max   x_center y_center radius
    z = Legendre # Axial in z
    disc = Zernike # (r, t) default to unit disc in x-y plane
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
    output_sub_cycles = true
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
