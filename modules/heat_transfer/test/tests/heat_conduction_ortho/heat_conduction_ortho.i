#
# Three independent cubes are thermally loaded, one in x, one in y, and one in z.
# Each direction has a different thermal conductivity, resulting in a different
#   temperature at the side with the Neumann bc.
#
# For x: 100/1000 = 1e-1
# For y: 100/100  = 1e+0
# for z: 100/10   = 1e+1
#

[Mesh]
  file = heat_conduction_ortho.e
[]

[Variables]
  [./temperature]
  [../]
[]

[Kernels]
  [./heat]
    type = AnisoHeatConduction
    variable = temperature
  [../]
[]

[BCs]
  [./temperatures]
    type = DirichletBC
    variable = temperature
    boundary = 1
    value = 0
  [../]
  [./neum]
    type = NeumannBC
    variable = temperature
    boundary = 2
    value = 100
  [../]
[]

[Materials]
  [./heat]
    type = AnisoHeatConductionMaterial
    block = 1
    specific_heat = 0.116
    thermal_conductivity = '10.0 0 0 0 10.0 0 0 0 10.0'
    temperature = temperature
  [../]
  [./density]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'density'
    prop_values = 0.283
  [../]
[]

[Executioner]
  type = Steady

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'
  line_search = 'none'

  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-10
  l_max_its = 20
[]

[Outputs]
  exodus = true
  hide = 'tcx tcy tcz'
[]

[Postprocessors]
  [./tcx]
    type = FunctionValuePostprocessor
    function = 1000
    outputs = none
    execute_on = 'initial timestep_end'
  [../]
  [./tcy]
    type = FunctionValuePostprocessor
    function = 100
    outputs = none
    execute_on = 'initial timestep_end'
  [../]
  [./tcz]
    type = FunctionValuePostprocessor
    function = 10
    outputs = none
    execute_on = 'initial timestep_end'
  [../]
[]
