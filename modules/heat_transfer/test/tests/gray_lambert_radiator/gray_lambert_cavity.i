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
  [view_factors_uo]
    type = SpecifiedViewFactor
    boundary = 'bottom top left right'
    view_factors = '0 0.123 0.6928 0.1841;
                    0.123 0 0.1841 0.6928;
                    0.2771 0.0736 0.4458 0.2035;
                    0.0736 0.2771 0.2035 0.4458'
    normalize_view_factor = true
    normalization_method = inverse_row_sum
    view_factor_tol = 0.05
    # Without changing from 'TIMESTEP_END' to 'INITIAL' here, this test would
    # diff, since GrayLambertSurfaceRadiationBase objects currently set their
    # view factors in initial() (and finalize() of this view factor object has
    # not yet executed). See https://github.com/idaholab/moose/issues/32611.
    # Alternatively we can keep 'TIMESTEP_END' and do 'execution_order_group = -1'.
    execute_on = 'INITIAL'
  []
  [./gray_lambert]
    type = ViewFactorObjectSurfaceRadiation
    boundary = 'bottom top left right'
    fixed_temperature_boundary = 'bottom top'
    fixed_boundary_temperatures = '550 300'
    adiabatic_boundary = 'right left'
    emissivity = '1 0.75 0.75 0.75'
    temperature = temperature
    view_factor_object_name = view_factors_uo
  [../]
[]

[VectorPostprocessors]
  [./lambert_vpp]
    type = SurfaceRadiationVectorPostprocessor
    surface_radiation_object_name = gray_lambert
    information = 'temperature emissivity radiosity heat_flux_density'
  [../]

  [./view_factors]
    type = ViewFactorVectorPostprocessor
    view_factor_object_name = view_factors_uo
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
