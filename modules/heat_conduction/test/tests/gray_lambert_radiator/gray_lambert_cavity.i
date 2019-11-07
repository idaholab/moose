[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 2
  nx = 1
  ny = 1
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
    type = ConstantViewFactorSurfaceRadiation
    boundary = 'bottom top left right'
    fixed_temperature_boundary = 'bottom top'
    fixed_boundary_temperatures = '550 300'
    adiabatic_boundary = 'right left'
    emissivity = '1 0.75 0.75 0.75'
    temperature = temperature
    view_factors = '0 0.123 0.6928 0.1841;
                    0.123 0 0.1841 0.6928;
                    0.2771 0.0736 0.4458 0.2035;
                    0.0736 0.2771 0.2035 0.4458'
  [../]
[]

[VectorPostprocessors]
  [./lambert_vpp]
    type = SurfaceRadiationVectorPostprocessor
    surface_radiation_object_name = gray_lambert
    information = 'temperature emissivity radiosity heat_flux_density'
  [../]

  [./view_factors]
    type = ViewfactorVectorPostprocessor
    surface_radiation_object_name = gray_lambert
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
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
