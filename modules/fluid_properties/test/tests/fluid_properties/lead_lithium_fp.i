[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [temperature]
    initial_condition = 700
  []
  [pressure]
    initial_condition = 101325
  []
[]

[FluidProperties]
  [fp]
    type = LeadLithiumFluidProperties 
    k = k_from_p_T
  []
[]

[Materials]
  [pbli]
    type = FluidPropertiesMaterialPT
    fp = fp
    pressure = pressure
    temperature = temperature
    
    outputs = 'all'

    # output_properties = 'density k cp cv viscosity h' 
    # output_properties = 'density k cp' 
    output_properties = 'k' 
    
    compute_sound_speed = false
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Postprocessors]
  # [Density]
  #   type = ElementAverageValue
  #   variable = density
  # []
  [Thermal_Conductivity]
    type = ElementAverageValue
    variable = k
  []
[]

# [Executioner]
#   type = Steady
# []

[Outputs]
  csv = true
[]
