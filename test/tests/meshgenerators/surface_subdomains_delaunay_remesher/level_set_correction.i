level_set = 'x*x + y*y +z*z - 4'

[GlobalParams]
  execute_on = FINAL
[]

[Mesh/remesh]
  exclude_subdomain_names = '4'
  level_set = '${level_set}'
[]


[AuxVariables/level_set_eval]
[]

[AuxKernels/level_set_eval]
  type = ParsedAux
  variable = level_set_eval
  expression = '${level_set}'
  use_xyzt = true
[]

# Ensure the level set at nodes is near zero
[Postprocessors]
  [level_set_eval_min]
    type = NodalExtremeValue
    variable = level_set_eval
    value_type = min
  []
  [level_set_eval_max]
    type = NodalExtremeValue
    variable = level_set_eval
    value_type = max
  []
[]

[Problem]
  solve = False
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = True
[]
