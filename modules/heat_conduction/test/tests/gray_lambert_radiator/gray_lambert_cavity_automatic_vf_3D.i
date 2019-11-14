[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 2
  zmin = 0
  zmax = 3
  nx = 4
  ny = 4
  nz = 4
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./temperature]
    initial_condition = 300
  [../]
[]

[UserObjects]
  [./gray_lambert]
    type = ViewFactorObjectSurfaceRadiation
    boundary = 'bottom top left right front back'
    fixed_temperature_boundary = 'bottom top'
    fixed_boundary_temperatures = '550 300'
    adiabatic_boundary = 'right left front back'
    emissivity = '1 0.75 0.75 0.75 0.75 0.75'
    temperature = temperature
    view_factor_object_name = view_factor
  [../]

  [./view_factor]
    type = UnobstructedPlanarViewFactor
    boundary = 'bottom top left right front back'
    normalize_view_factor = true
    execute_on = 'INITIAL'
  [../]
[]

[Postprocessors]
  [./heat_flux_density_bottom]
    type = GrayLambertSurfaceRadiationPP
    surface_radiation_object_name = gray_lambert
    return_type = HEAT_FLUX_DENSITY
    boundary = bottom
  [../]

  [./temperature_left]
    type = GrayLambertSurfaceRadiationPP
    surface_radiation_object_name = gray_lambert
    return_type = TEMPERATURE
    boundary = left
  [../]

  [./temperature_right]
    type = GrayLambertSurfaceRadiationPP
    surface_radiation_object_name = gray_lambert
    return_type = TEMPERATURE
    boundary = right
  [../]

  [./brightness_top]
    type = GrayLambertSurfaceRadiationPP
    surface_radiation_object_name = gray_lambert
    return_type = RADIOSITY
    boundary = top
  [../]

  [./brightness_front]
    type = GrayLambertSurfaceRadiationPP
    surface_radiation_object_name = gray_lambert
    return_type = RADIOSITY
    boundary = front
  [../]

  [./brightness_back]
    type = GrayLambertSurfaceRadiationPP
    surface_radiation_object_name = gray_lambert
    return_type = RADIOSITY
    boundary = back
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
[]
