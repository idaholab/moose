[Mesh]
  [msh]
    type = FileMeshGenerator
    file = 'Blanket_mesh.msh'
  []
[]

[Outputs]
  exodus = true
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type --pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Variables]
  [temp]
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temp
  []
  [heat_generation]
    type = CoupledForce
    variable = temp
    v = power_density
  []
[]

[BCs]
  #[LeftBC]
    #type = NeumannBC
    #variable = temp
    #boundary = 'left'
    #value = 1
  #[]
  #[RightBC]
    #type = DirichletBC
    #variable = temp
    #boundary = 'right'
    #value = 1
  #[]
[]

[Materials]
  [hcm]
    type = HeatConductionMaterial
    specific_heat = 1
    thermal_conductivity = 1
  []
[]

[AuxVariables]
  [power_density]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [power]
    type = FNSFSourceAux
    variable = power_density
    inner_xi = '-30 -15 15 30'
    outer_xi = '-35 -20 20 35'
    depth = '0.46 1 1.4'
    source = '1e5 1e5 1e5 1e6 1e6 1e6'
  []
[]
