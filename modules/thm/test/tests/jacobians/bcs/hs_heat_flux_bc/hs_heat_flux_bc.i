[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [T]
    initial_condition = 300
  []
[]

[BCs]
  [bc]
    type = HSHeatFluxBC
    variable = T
    boundary = 0
    function = 5
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_test_jacobian'
  petsc_options_iname = '-snes_test_error'
  petsc_options_value = '1e-8'
[]
