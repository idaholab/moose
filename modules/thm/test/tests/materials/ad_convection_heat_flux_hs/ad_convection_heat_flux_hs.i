# Gold value should be the following:
#   q_wall = kappa * htc_wall * (T_wall - T)
#          = 0.5 * 100 * (500 - 400)
#          = 5000

[GlobalParams]
  execute_on = 'initial'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Materials]
  [props]
    type = ADGenericConstantMaterial
    prop_names = 'T T_wall htc_wall kappa'
    prop_values = '400 500 100 0.5'
  []
  [q_wall_mat]
    type = ADConvectionHeatFluxHSMaterial
    q_wall = q_wall_prop
    T = T
    T_wall = T_wall
    htc_wall = htc_wall
    kappa = kappa
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [q_wall_pp]
    type = ADElementAverageMaterialProperty
    mat_prop = q_wall_prop
  []
[]

[Outputs]
  csv = true
[]
