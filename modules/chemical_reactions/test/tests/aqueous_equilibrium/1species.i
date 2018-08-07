# Simple equilibrium reaction example to illustrate the use of the AqueousEquilibriumReactions
# action.
# In this example, a single primary species a is transported by diffusion and convection
# from the left of the porous medium, reacting to form an equilibrium species pa2 according to
# the equilibrium reaction specified in the AqueousEquilibriumReactions block as:
#
#  reactions = '2a = pa2 1'
#
# where the 2 is the weight of the equilibrium species, and the 1 refers to the equilibrium
# constant (log10(Keq) = 1).
#
# The AqueousEquilibriumReactions action creates all the required kernels and auxkernels
# to compute the reaction given by the above equilibrium reaction equation.
#
# Specifically, it adds to following:
# * An AuxVariable named 'pa2' (given in the reactions equations)
# * A AqueousEquilibriumRxnAux AuxKernel for this AuxVariable with all parameters
# * A CoupledBEEquilibriumSub Kernel for each primary species with all parameters
# * A CoupledDiffusionReactionSub Kernel for each primary species with all parameters
# * A CoupledConvectionReactionSub Kernel for each primary species with all parameters if
# pressure is a coupled variable

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
[]

[ICs]
  [./pressure]
    type = FunctionIC
    variable = pressure
    function = 2-x
  [../]
[]

[ReactionNetwork]
  [./AqueousEquilibriumReactions]
    primary_species = a
    reactions = '2a = pa2 1'
    secondary_species = pa2
    pressure = pressure
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
