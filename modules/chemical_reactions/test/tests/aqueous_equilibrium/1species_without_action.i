# Simple equilibrium reaction example.
# This simulation is identical to 1species.i, but explicitly includes the AuxVariables,
# AuxKernels, and Kernels that the action in 1species.i adds

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
[]

[Variables]
  [./a]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = BoundingBoxIC
      x1 = 0.0
      y1 = 0.0
      x2 = 1e-2
      y2 = 1
      inside = 1.0e-2
      outside = 1.0e-10
      variable = a
    [../]
  [../]
[]

[AuxVariables]
  [./pressure]
    order = FIRST
    family = LAGRANGE
  [../]
  [./pa2]
  [../]
[]

[AuxKernels]
  [./pa2eq]
    type = AqueousEquilibriumRxnAux
    variable = pa2
    v = a
    sto_v = 2
    log_k = 1
  [../]
[]

[ICs]
  [./pressure]
    type = FunctionIC
    variable = pressure
    function = 2-x
  [../]
[]

[Kernels]
  [./a_ie]
    type = PrimaryTimeDerivative
    variable = a
  [../]
  [./a_diff]
    type = PrimaryDiffusion
    variable = a
  [../]
  [./a_conv]
    type = PrimaryConvection
    variable = a
    p = pressure
  [../]
  [./aeq]
    type = CoupledBEEquilibriumSub
    variable = a
    log_k = 1
    weight = 2
    sto_u = 2
  [../]
  [./adiff]
    type = CoupledDiffusionReactionSub
    variable = a
    log_k = 1
    weight = 2
    sto_u = 2
  [../]
  [./aconv]
    type = CoupledConvectionReactionSub
    variable = a
    log_k = 1
    weight = 2
    sto_u = 2
    p = pressure
  [../]
[]

[BCs]
  [./a_right]
    type = ChemicalOutFlowBC
    variable = a
    boundary = right
  [../]
[]

[Materials]
  [./porous]
    type = GenericConstantMaterial
    prop_names = 'diffusivity conductivity porosity'
    prop_values = '1e-4 1e-4 0.2'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-12
  start_time = 0.0
  end_time = 100
  dt = 10.0
[]

[Outputs]
  file_base = 1species_out
  exodus = true
  perf_graph = true
  print_linear_residuals = true
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]
