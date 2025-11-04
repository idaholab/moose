!include sod_shock_tube.i

[Components]
  [pipe]
    wave_speed_formulation = davis
  []
[]

[Executioner]
  [TimeIntegrator]
    order = 1
  []
  num_steps := 5
[]

[VectorPostprocessors]
  [vpp]
    type = ElementValueSampler
    variable = rho
    sort_by = x
    execute_on = 'FINAL'
  []
[]

[Outputs]
  file_base := davis_wave_speed
  [csv]
    type = CSV
    execute_vector_postprocessors_on = 'FINAL'
    create_final_symlink = true
  []
  [out]
    enable = false
  []
[]
