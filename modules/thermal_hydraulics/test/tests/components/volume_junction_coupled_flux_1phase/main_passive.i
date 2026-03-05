# Here we add a passive species with concentration C = 1 kg/m^3 to the flow.

!include main.i

[Components]
  [pipe]
    passives_names = 'C'
    initial_passives = '${fparse 0.5 / A_pipe}'
  []
  [junction]
    initial_passives = '${fparse 0.5 / A_pipe}'
  []
[]

[MultiApps]
  [child]
    input_files := child_passive.i
  []
[]
