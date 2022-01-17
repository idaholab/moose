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

[BCs]
  [bc]
    type = RadiativeHeatFluxRZBC
    variable = T
    boundary = 2
    Tinfinity = 1500
    boundary_emissivity = 0.3
    view_factor = 0.5
    axis_point = '0 0 0'
    axis_dir = '1 0 0'
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
