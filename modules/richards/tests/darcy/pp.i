# investigating pressure pulse in 1D
# transient

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 100
[]

[GlobalParams]
  variable = pressure
  fluid_weight = '0 0 0'
  fluid_viscosity = 1E-3
[]


[Variables]
  [./pressure]
    order = FIRST
    family = LAGRANGE
    initial_condition = 2E6
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    boundary = left
    value = 3E6
  [../]
[]

[Kernels]
  [./time_deriv]
    type = TimeDerivative
  [../]
  [./darcy]
    type = DarcyFlux
  [../]
[]

[AuxVariables]
  [./f_0]
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
[]

[Materials]
  [./solid]
    type = DarcyMaterial
    block = 0
    mat_permeability = '2E-5 0 0  0 2E-5 0  0 0 2E-5' # this is the permeability (1E-15) multiplied by the bulk modulus (2E9) divided by the porosity (0.1)
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  petsc_options_iname = '-ksp_type -pc_type'
  petsc_options_value = 'bcgs bjacobi'
  dt = 1E3
  end_time = 1E4
[]

[Outputs]
  file_base = pp
  execute_on = 'timestep_end final'
  interval = 10000
  exodus = true
[]
