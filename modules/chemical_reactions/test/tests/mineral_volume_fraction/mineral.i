# Test the MineralVolumeFraction postprocessor

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 2
  ymax = 2
[]

[Variables]
  [./mineral_conc]
    initial_condition = 0.1
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [./volume_frac]
    type = TotalMineralVolumeFraction
    variable = mineral_conc
    molar_volume = 20
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  csv = true
[]
