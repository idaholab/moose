[Mesh]
  file = gmsh_mesh.msh
[]

[Variables]
  [./u]
    block = 6
  [../]

  [./v]
    block = 5
  [../]
[]

[Kernels]
  [./diff_u]
    type = CoeffParamDiffusion
    variable = u
    D = 4
    block = 6
  [../]
  [./diff_v]
    type = CoeffParamDiffusion
    variable = v
    D = 2
    block = 5
  [../]
  [./source_u]
    type = BodyForce
    variable = u
    value = 1
  [../]
[]

[InterfaceKernels]
 [./interface]
   type = PenaltyInterfaceDiffusion
   variable = u
   neighbor_var = v
   boundary = '1 2'
   penalty = 1e6
 [../]
[]

[BCs]
  [./u]
    type = VacuumBC
    variable = u
    boundary = 4
  [../]
  [./v]
    type = VacuumBC
    variable = v
    boundary = 3
  [../]
[]

[Postprocessors]
  [./u_int]
    type = ElementIntegralVariablePostprocessor
    variable = u
    block = 6
  [../]
  [./v_int]
    type = ElementIntegralVariablePostprocessor
    variable = v
    block = 5
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]
