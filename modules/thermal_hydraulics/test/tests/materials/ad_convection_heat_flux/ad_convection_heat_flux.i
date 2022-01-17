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

[AuxVariables]
  [T_wall]
  []
[]

[AuxKernels]
  [T_wall_ak]
    type = ConstantAux
    variable = T_wall
    value = 500
  []
[]

[Materials]
  [props]
    type = ADGenericConstantMaterial
    prop_names = 'T htc_wall kappa'
    prop_values = '400 100 0.5'
  []
  [q_wall_mat]
    type = ADConvectionHeatFluxMaterial
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
