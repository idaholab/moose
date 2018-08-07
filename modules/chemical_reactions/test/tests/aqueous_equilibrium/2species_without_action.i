# Simple equilibrium reaction example to illustrate the use of the AqueousEquilibriumReactions
# action.
# In this example, two primary species a and b are transported by diffusion and convection
# from the left of the porous medium, reacting to form two equilibrium species pa2 and pab
# according to the equilibrium reaction specified in the AqueousEquilibriumReactions block as:
#
#      reactions = '2a = pa2     2
#                   a + b = pab -2'
#
# where the 2 is the weight of the equilibrium species, the 2 on the RHS of the first reaction
# refers to the equilibrium constant (log10(Keq) = 2), and the -2 on the RHS of the second
# reaction equates to log10(Keq) = -2.
#
# This example is identical to 2species.i, except that it explicitly includes all AuxKernels
# and Kernels that are set up by the action in 2species.i

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
      x2 = 1.0e-10
      y2 = 1
      inside = 1.0e-2
      outside = 1.0e-10
    [../]
  [../]
  [./b]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = BoundingBoxIC
      x1 = 0.0
      y1 = 0.0
      x2 = 1.0e-10
      y2 = 1
      inside = 1.0e-2
      outside = 1.0e-10
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
  [./pab]
  [../]
[]

[AuxKernels]
  [./pa2eq]
    type = AqueousEquilibriumRxnAux
    variable = pa2
    v = a
    sto_v = 2
    log_k = 2
  [../]
  [./pabeq]
    type = AqueousEquilibriumRxnAux
    variable = pab
    v = 'a b'
    sto_v = '1 1'
    log_k = -2
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
  [./b_ie]
    type = PrimaryTimeDerivative
    variable = b
  [../]
  [./b_diff]
    type = PrimaryDiffusion
    variable = b
  [../]
  [./b_conv]
    type = PrimaryConvection
    variable = b
    p = pressure
  [../]
  [./a1eq]
    type = CoupledBEEquilibriumSub
    variable = a
    log_k = 2
    weight = 2
    sto_u = 2
  [../]
  [./a1diff]
    type = CoupledDiffusionReactionSub
    variable = a
    log_k = 2
    weight = 2
    sto_u = 2
  [../]
  [./a1conv]
    type = CoupledConvectionReactionSub
    variable = a
    log_k = 2
    weight = 2
    sto_u = 2
    p = pressure
  [../]
  [./a2eq]
    type = CoupledBEEquilibriumSub
    variable = a
    v = b
    log_k = -2
    weight = 1
    sto_v = 1
    sto_u = 1
  [../]
  [./a2diff]
    type = CoupledDiffusionReactionSub
    variable = a
    v = b
    log_k = -2
    weight = 1
    sto_v = 1
    sto_u = 1
  [../]
  [./a2conv]
    type = CoupledConvectionReactionSub
    variable = a
    v = b
    log_k = -2
    weight = 1
    sto_v = 1
    sto_u = 1
    p = pressure
  [../]
  [./b2eq]
    type = CoupledBEEquilibriumSub
    variable = b
    v = a
    log_k = -2
    weight = 1
    sto_v = 1
    sto_u = 1
  [../]
  [./b2diff]
    type = CoupledDiffusionReactionSub
    variable = b
    v = a
    log_k = -2
    weight = 1
    sto_v = 1
    sto_u = 1
  [../]
  [./b2conv]
    type = CoupledConvectionReactionSub
    variable = b
    v = a
    log_k = -2
    weight = 1
    sto_v = 1
    sto_u = 1
    p = pressure
  [../]
[]

[BCs]
  [./a_left]
    type = DirichletBC
    variable = a
    boundary = left
    value = 1.0e-2
  [../]
  [./a_right]
    type = ChemicalOutFlowBC
    variable = a
    boundary = right
  [../]
  [./b_left]
    type = DirichletBC
    variable = b
    boundary = left
    value = 1.0e-2
  [../]
  [./b_right]
    type = ChemicalOutFlowBC
    variable = b
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
  file_base = 2species_out
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
