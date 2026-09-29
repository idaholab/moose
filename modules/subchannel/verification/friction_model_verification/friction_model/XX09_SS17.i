# EBR-II XX09 SHRT-17 steady state of validation/EBR-II/XX09_SCM_SS17.i, with the mass flow rate
# and the position of the TTC subchannels at the TTC height for the comparison with DASSH.
# The default friction closure is UCTD. For PCTD run:
# subchannel-opt -i XX09_SS17.i SCMClosures/Chen/friction_model=Pacio Outputs/file_base=XX09_SS17_pacio_out

!include ../../../validation/EBR-II/XX09_SCM_SS17.i

# SCMTriPowerIC opens the file relative to the working directory
[ICs]
  [q_prime_IC]
    filename := '../../../validation/EBR-II/pin_power_profile61_uniform.txt'
  []
[]

# The projection onto a detailed mesh is not needed
[MultiApps]
  active = ''
[]

[Transfers]
  active = ''
[]

[Functions]
  [x_fn]
    type = ParsedFunction
    expression = 'x'
  []
  [y_fn]
    type = ParsedFunction
    expression = 'y'
  []
[]

[AuxVariables]
  [x_sc]
    block = subchannel
  []
  [y_sc]
    block = subchannel
  []
[]

[AuxKernels]
  [x_sc]
    type = FunctionAux
    variable = x_sc
    function = x_fn
    block = subchannel
    execute_on = 'initial'
  []
  [y_sc]
    type = FunctionAux
    variable = y_sc
    function = y_fn
    block = subchannel
    execute_on = 'initial'
  []
[]

# Same subchannels and height as the TTC temperature postprocessors
[Postprocessors]
  [mdot-27]
    type = SubChannelPointValue
    variable = mdot
    index = 91
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [mdot-28]
    type = SubChannelPointValue
    variable = mdot
    index = 50
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [mdot-29]
    type = SubChannelPointValue
    variable = mdot
    index = 21
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [mdot-30]
    type = SubChannelPointValue
    variable = mdot
    index = 4
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [mdot-31]
    type = SubChannelPointValue
    variable = mdot
    index = 2
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [mdot-32]
    type = SubChannelPointValue
    variable = mdot
    index = 16
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [mdot-33]
    type = SubChannelPointValue
    variable = mdot
    index = 42
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [mdot-34]
    type = SubChannelPointValue
    variable = mdot
    index = 80
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [mdot-35]
    type = SubChannelPointValue
    variable = mdot
    index = 107
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [x-27]
    type = SubChannelPointValue
    variable = x_sc
    index = 91
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [x-28]
    type = SubChannelPointValue
    variable = x_sc
    index = 50
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [x-29]
    type = SubChannelPointValue
    variable = x_sc
    index = 21
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [x-30]
    type = SubChannelPointValue
    variable = x_sc
    index = 4
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [x-31]
    type = SubChannelPointValue
    variable = x_sc
    index = 2
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [x-32]
    type = SubChannelPointValue
    variable = x_sc
    index = 16
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [x-33]
    type = SubChannelPointValue
    variable = x_sc
    index = 42
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [x-34]
    type = SubChannelPointValue
    variable = x_sc
    index = 80
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [x-35]
    type = SubChannelPointValue
    variable = x_sc
    index = 107
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [y-27]
    type = SubChannelPointValue
    variable = y_sc
    index = 91
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [y-28]
    type = SubChannelPointValue
    variable = y_sc
    index = 50
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [y-29]
    type = SubChannelPointValue
    variable = y_sc
    index = 21
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [y-30]
    type = SubChannelPointValue
    variable = y_sc
    index = 4
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [y-31]
    type = SubChannelPointValue
    variable = y_sc
    index = 2
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [y-32]
    type = SubChannelPointValue
    variable = y_sc
    index = 16
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [y-33]
    type = SubChannelPointValue
    variable = y_sc
    index = 42
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [y-34]
    type = SubChannelPointValue
    variable = y_sc
    index = 80
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
  [y-35]
    type = SubChannelPointValue
    variable = y_sc
    index = 107
    height = 0.322
    execute_on = 'TIMESTEP_END'
  []
[]
