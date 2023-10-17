[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -5.0
  xmax = 5.0
  nx = 10

  ymin = -5.0
  ymax = 5.0
  ny = 10

  zmin = 0.0
  zmax = 1.0
  nz = 1
[]

[Variables]
  [./temp]
    initial_condition = 300
  [../]
[]

[Kernels]
  [./time]
    type = ADHeatConductionTimeDerivative
    variable = temp
  [../]
  [./heat_conduct]
    type = ADHeatConduction
    variable = temp
    thermal_conductivity = thermal_conductivity
  [../]
  [./heat_source]
    type = ADMatHeatSource
    material_property = volumetric_heat
    variable = temp
  [../]
[]

[BCs]
  [./temp_bottom_fix]
    type = ADDirichletBC
    variable = temp
    boundary = 1
    value = 300
  [../]
[]

[Materials]
  [./heat]
    type = ADHeatConductionMaterial
    specific_heat = 603
    thermal_conductivity = 10e-2
  [../]
  [./density]
    type = ADGenericConstantMaterial
    prop_names = 'density'
    prop_values = '4.43e-6'
  [../]
  [./volumetric_heat]
    type = FunctionPathEllipsoidHeatSource
    rx = 1
    ry = 1
    rz = 1
    power = 1000
    efficiency = 0.5
    factor = 2
    function_x= path_x
    function_y= path_y
    function_z= path_z
  [../]
[]

[Functions]
  [./path_x]
    type = ParsedFunction
    expression = 2*cos(2.0*pi*t)
  [../]
  [./path_y]
    type = ParsedFunction
    expression = 2*sin(2.0*pi*t)
  [../]
  [./path_z]
    type = ParsedFunction
    expression = 1.0
  [../]
[]

[Postprocessors]
  [temp_max]
    type = ElementExtremeValue
    variable = temp
  []
  [temp_min]
    type = ElementExtremeValue
    variable = temp
    value_type = min
  []
  [temp_avg]
    type = ElementAverageValue
    variable = temp
  []
[]

[Preconditioning]
  [./full]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK

  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6

  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'preonly lu       superlu_dist'

  l_max_its = 100

  end_time = 1
  dt = 0.1
  dtmin = 1e-4
[]

[Outputs]
  csv = true
[]
