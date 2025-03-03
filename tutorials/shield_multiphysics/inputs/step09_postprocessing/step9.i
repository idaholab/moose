!include ../step08_adaptivity/step8_adapt.i

[Postprocessors]
  [num_elements]
    type = NumElements
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_temperature_concrete]
    type = NodalExtremeValue
    variable = T
    block = 'concrete_hd concrete'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [water_heat_flux]
    type = ADSideDiffusiveFluxIntegral
    variable = T
    boundary = water_boundary_inwards
    diffusivity = 'thermal_conductivity'
  []
[]

[VectorPostprocessors]
  [temperature_sample_x]
    type = LineValueSampler
    num_points = 50
    start_point = '1.275 4.625 0.8'
    end_point = '5.275 4.625 0.8'
    variable = T
    sort_by = x
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [temperature_sample_y]
    type = LineValueSampler
    num_points = 50
    start_point = '3.275 0.825 0.8'
    end_point = '3.275 8.425 0.8'
    variable = T
    sort_by = y
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
[]
