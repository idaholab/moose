# Test to show that DarcyFlux produces the correct jacobian

[GlobalParams]
  variable = pressure
  fluid_weight = '0 0 -1.5'
  fluid_viscosity = 1
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[Variables]
  [./pressure]
    [./InitialCondition]
      type = RandomIC
      block = 0
      min = 0
      max = 1
    [../]
  [../]
[]

[Kernels]
  [./darcy]
    type = DarcyFlux
    variable = pressure
  [../]
[]

[Materials]
  [./solid]
    type = DarcyMaterial
    block = 0
    mat_permeability = '1 0 0  0 2 0  0 0 3'
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-snes_type'
    petsc_options_value = 'test'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = Newton
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = jac
  exodus = false
[]
