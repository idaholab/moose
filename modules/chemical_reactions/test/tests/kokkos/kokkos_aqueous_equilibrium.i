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
  [a]
    order = FIRST
    family = LAGRANGE
    [InitialCondition]
      type = BoundingBoxIC
      x1 = 0.0
      y1 = 0.0
      x2 = 1.0e-10
      y2 = 1
      inside = 1.0e-2
      outside = 1.0e-10
    []
  []
  [b]
    order = FIRST
    family = LAGRANGE
    [InitialCondition]
      type = BoundingBoxIC
      x1 = 0.0
      y1 = 0.0
      x2 = 1.0e-10
      y2 = 1
      inside = 1.0e-2
      outside = 1.0e-10
    []
  []
[]

[AuxVariables]
  [pressure]
    order = FIRST
    family = LAGRANGE
  []
  [pa2]
  []
  [pab]
  []
[]

[AuxKernels]
  [pa2eq]
    type = KokkosAqueousEquilibriumRxnAux
    variable = pa2
    v = a
    sto_v = 2
    log_k = 2
  []
  [pabeq]
    type = KokkosAqueousEquilibriumRxnAux
    variable = pab
    v = 'a b'
    sto_v = '1 1'
    log_k = -2
  []
[]

[ICs]
  [pressure]
    type = FunctionIC
    variable = pressure
    function = 2-x
  []
[]

[Kernels]
  [a_ie]
    type = KokkosPrimaryTimeDerivative
    variable = a
  []
  [a_diff]
    type = KokkosPrimaryDiffusion
    variable = a
  []
  [a_conv]
    type = KokkosPrimaryConvection
    variable = a
    p = pressure
  []
  [b_ie]
    type = KokkosPrimaryTimeDerivative
    variable = b
  []
  [b_diff]
    type = KokkosPrimaryDiffusion
    variable = b
  []
  [b_conv]
    type = KokkosPrimaryConvection
    variable = b
    p = pressure
  []
  [a1eq]
    type = KokkosCoupledBEEquilibriumSub
    variable = a
    log_k = 2
    weight = 2
    sto_u = 2
  []
  [a1diff]
    type = KokkosCoupledDiffusionReactionSub
    variable = a
    log_k = 2
    weight = 2
    sto_u = 2
  []
  [a1conv]
    type = KokkosCoupledConvectionReactionSub
    variable = a
    log_k = 2
    weight = 2
    sto_u = 2
    p = pressure
  []
  [a2eq]
    type = KokkosCoupledBEEquilibriumSub
    variable = a
    v = b
    log_k = -2
    weight = 1
    sto_v = 1
    sto_u = 1
  []
  [a2diff]
    type = KokkosCoupledDiffusionReactionSub
    variable = a
    v = b
    log_k = -2
    weight = 1
    sto_v = 1
    sto_u = 1
  []
  [a2conv]
    type = KokkosCoupledConvectionReactionSub
    variable = a
    v = b
    log_k = -2
    weight = 1
    sto_v = 1
    sto_u = 1
    p = pressure
  []
  [b2eq]
    type = KokkosCoupledBEEquilibriumSub
    variable = b
    v = a
    log_k = -2
    weight = 1
    sto_v = 1
    sto_u = 1
  []
  [b2diff]
    type = KokkosCoupledDiffusionReactionSub
    variable = b
    v = a
    log_k = -2
    weight = 1
    sto_v = 1
    sto_u = 1
  []
  [b2conv]
    type = KokkosCoupledConvectionReactionSub
    variable = b
    v = a
    log_k = -2
    weight = 1
    sto_v = 1
    sto_u = 1
    p = pressure
  []
[]

[BCs]
  [a_left]
    type = KokkosDirichletBC
    variable = a
    boundary = left
    value = 1.0e-2
  []
  [a_right]
    type = KokkosChemicalOutFlowBC
    variable = a
    boundary = right
  []
  [b_left]
    type = KokkosDirichletBC
    variable = b
    boundary = left
    value = 1.0e-2
  []
  [b_right]
    type = KokkosChemicalOutFlowBC
    variable = b
    boundary = right
  []
[]

[Materials]
  [porous]
    type = KokkosGenericConstantMaterial
    prop_names = 'diffusivity conductivity porosity'
    prop_values = '1e-4 1e-4 0.2'
  []
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
  exodus = true
  perf_graph = true
  print_linear_residuals = true
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]
