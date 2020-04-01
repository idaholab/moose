[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./u]
  [../]
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [./km_to_m]
    type = FunctionValuePostprocessor
    function = ${units 1 km -> m}
  [../]
  [./Jmol_to_eVat]
    type = FunctionValuePostprocessor
    function = ${units 1 J/mol -> eV/at}
  [../]
  [./mW]
    type = FunctionValuePostprocessor
    function = ${units 3 mW}
  [../]
[]

[Outputs]
  csv = true
[]
