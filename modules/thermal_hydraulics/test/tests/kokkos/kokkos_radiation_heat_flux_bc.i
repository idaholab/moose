[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [T]
    initial_condition = 1000
  []
[]

[KokkosBCs]
  [bc]
    type = KokkosRadiativeHeatFluxBC
    variable = T
    boundary = 0
    Tinfinity = 1500
    boundary_emissivity = 0.3
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
