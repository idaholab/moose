[Mesh]
  type = GeneratedMesh
  dim = 2 
  nx = 10
  ny = 10
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
  #line_search = 'none'
  #l_tol = 1e+09
  #l_max_its = 2e+08
  #nl_rel_tol = 1e+09
  #automatic_scaling = true
  #[./Quadrature]
    #allow_negative_qweights = false
  #[../]
[]

[Variables]
  [temp]
    #initial_condition = 10
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
  [LeftBC]
    type = NeumannBC
    variable = temp
    boundary = 'left'
    value = 1
  []
  [RightBC]
    type = DirichletBC
    variable = temp
    boundary = 'right'
    value = 1
  []
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
    inner_xi = '-67.1621 -45.0782 -26.682 -8.91958 8.91958 26.682 45.0782 67.1621'
    outer_xi = '-80.7101 -53.2501 -29.8105 -9.58303 9.58303 29.8105 53.2501 80.7101'
    depth = '0.1 0.102 0.14 0.163 0.181 0.199 0.217 0.277 0.295 0.321 0.339 0.443 0.461 0.531 0.549 0.652 0.67 0.82 0.838 1.08 1.1'
    source = '2.7544e+07 2.7544e+07 2.7544e+07 2.7544e+07 2.7544e+07 2.7544e+07 2.7544e+07 4.6228e+06 4.6228e+06 4.6228e+06 4.6228e+06 4.6228e+06 4.6228e+06 4.6228e+06 6.4374e+06 6.4374e+06 6.4374e+06 6.4374e+06 6.4374e+06 6.4374e+06 6.4374e+06 6.3422e+06 6.3422e+06 6.3422e+06 6.3422e+06 6.3422e+06 6.3422e+06 6.3422e+06 1.6260e+07 1.6260e+07 1.6260e+07 1.6260e+07 1.6260e+07 1.6260e+07 1.6260e+07 4.2483e+06 4.2483e+06 4.2483e+06 4.2483e+06 4.2483e+06 4.2483e+06 4.2483e+06 2.7470e+06 2.7470e+06 2.7470e+06 2.7470e+06 2.7470e+06 2.7470e+06 2.7470e+06 2.6035e+06 2.6035e+06 2.6035e+06 2.6035e+06 2.6035e+06 2.6035e+06 2.6035e+06 8.3437e+06 8.3437e+06 8.3437e+06 8.3437e+06 8.3437e+06 8.3437e+06 8.3437e+06 1.5133e+06 1.5133e+06 1.5133e+06 1.5133e+06 1.5133e+06 1.5133e+06 1.5133e+06 9.5635e+05 9.5635e+05 9.5635e+05 9.5635e+05 9.5635e+05 9.5635e+05 9.5635e+05 7.4320e+05 7.4320e+05 7.4320e+05 7.4320e+05 7.4320e+05 7.4320e+05 7.4320e+05 1.0826e+06 1.0826e+06 1.0826e+06 1.0826e+06 1.0826e+06 1.0826e+06 1.0826e+06 1.3815e+05 1.3815e+05 1.3815e+05 1.3815e+05 1.3815e+05 1.3815e+05 1.3815e+05 2.7832e+05 2.7832e+05 2.7832e+05 2.7832e+05 2.7832e+05 2.7832e+05 2.7832e+05 5.2306e+04 5.2306e+04 5.2306e+04 5.2306e+04 5.2306e+04 5.2306e+04 5.2306e+04 1.2672e+05 1.2672e+05 1.2672e+05 1.2672e+05 1.2672e+05 1.2672e+05 1.2672e+05 2.8766e+04 2.8766e+04 2.8766e+04 2.8766e+04 2.8766e+04 2.8766e+04 2.8766e+04 1.0178e+05 1.0178e+05 1.0178e+05 1.0178e+05 1.0178e+05 1.0178e+05 1.0178e+05 2.7972e+04 2.7972e+04 2.7972e+04 2.7972e+04 2.7972e+04 2.7972e+04 2.7972e+04'
  []
[]
