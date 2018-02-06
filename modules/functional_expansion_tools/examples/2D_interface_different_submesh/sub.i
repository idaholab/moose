# Derived from the example '2D_interface' with the following differences:
#
#   1) The number of y divisions in the sub app is not the same as the master app
#   2) The subapp mesh is skewed in y
#   3) The Functional Expansion order for the flux term was increased to 7
[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0.4
  xmax = 2.4
  nx = 30
  ymin = 0.0
  ymax = 10.0
  ny = 23
  bias_y = 1.2
[]

[Variables]
  [./s]
  [../]
[]

[Kernels]
  [./diff_s]
    type = HeatConduction
    variable = s
  [../]
  [./time_diff_s]
    type = HeatConductionTimeDerivative
    variable = s
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
  [./start_s]
    type = ConstantIC
    value = 2
    variable = s
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = s
    boundary = bottom
    value = 0.1
  [../]
  [./interface_flux]
    type = FXFluxBC
    boundary = left
    variable = s
    function = FX_Basis_Flux_Sub
  [../]
[]

[Functions]
  [./FX_Basis_Value_Sub]
    type = FunctionSeries
    series_type = Cartesian
    orders = '4'
    physical_bounds = '0.0 10'
    y = Legendre
  [../]
  [./FX_Basis_Flux_Sub]
    type = FunctionSeries
    series_type = Cartesian
    orders = '7'
    physical_bounds = '0.0 10'
    y = Legendre
  [../]
[]

[UserObjects]
  [./FX_Value_UserObject_Sub]
    type = FXBoundaryValueUserObject
    function = FX_Basis_Value_Sub
    variable = s
    boundary = left
  [../]
  [./FX_Flux_UserObject_Sub]
    type = FXBoundaryFluxUserObject
    function = FX_Basis_Flux_Sub
    variable = s
    boundary = left
    diffusivity = thermal_conductivity
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 1.0
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]
