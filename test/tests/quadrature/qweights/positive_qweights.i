[Mesh]
  [./square]
    type = FileMeshGenerator
    file = cube.e
  [../]
[]

[Variables]
  [u][]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [heat_source_fuel]
    type = CoupledForce
    variable = u
    v = power_density
  []
[]

[BCs]
  [robin]
    type = RobinBC
    variable = u
    boundary = '1 2 3 4 5 6'
  []
[]

[AuxVariables]
  [power_density]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [source]
    type = ParsedAux
    variable = power_density
    use_xyzt = true
    expression = 'if(x>0.1,100,1)'
  []
[]

[Executioner]
  type = Steady
  [./Quadrature]
    allow_negative_qweights = false
  [../]
  solve_type = 'NEWTON'
  petsc_options_iname = "-pc_type"
  petsc_options_value = "hypre"
[]

[Outputs]
  exodus = true
[]
