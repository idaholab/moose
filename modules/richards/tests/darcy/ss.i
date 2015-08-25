# Test to show that DarcyFlux produces the correct steadystate

[GlobalParams]
  variable = pressure
  fluid_weight = '0 0 -1E4'
  fluid_viscosity = 2
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 2
  nz = 3
  zmax = 0
  zmin = -10
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


[AuxVariables]
  [./f_0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f_1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f_2]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./f_0]
    type = DarcyFluxComponent
    component = x
    variable = f_0
    porepressure = pressure
  [../]
  [./f_1]
    type = DarcyFluxComponent
    component = y
    variable = f_1
    porepressure = pressure
  [../]
  [./f_2]
    type = DarcyFluxComponent
    component = z
    variable = f_2
    porepressure = pressure
  [../]
[]

[BCs]
  [./zmax]
    type = DirichletBC
    boundary = front
    value = 0
    variable = pressure
  [../]
[]


[Materials]
  [./solid]
    type = DarcyMaterial
    block = 0
    mat_permeability = '1E-5 0 0  0 1E-5 0  0 0 1E-5'
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = Newton
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = ss
  exodus = true
[]
