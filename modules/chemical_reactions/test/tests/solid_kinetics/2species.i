# Simple reaction-diffusion example to illustrate the use of the SolidKineticReactions
# action.
# In this example, two primary species a and b diffuse towards each other from
# opposite ends of a porous medium, reacting when they meet to form a mineral
# precipitate. The kinetic reaction is specified in the SolidKineticReactions block as:
#
# kin_reactions = '(1.0)a+(1.0)b=mineral'
#
# where a and b are the primary species (reactants), mineral is the precipitate,
# and the values in the parentheses are the stoichiometric coefficients for each
# species in the kinetic reaction.
#
# The SolidKineticReactions action creates all the required kernels and auxkernels
# to compute the reaction given by the above kinetic reaction equation.
#
# Specifically, it adds to following:
# * An AuxVariable named 'mineral' (given in the RHS of the kinetic reaction)
# * A KineticDisPreConcAux AuxKernel for this AuxVariable with all parameters
# * A CoupledBEKinetic Kernel for each primary species with all parameters

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 1
  ymax = 1
  nx = 40
[]

[Variables]
  [./a]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0
  [../]
  [./b]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0
  [../]
[]

[ReactionNetwork]
  [./SolidKineticReactions]
    primary_species = 'a b'
    secondary_species = mineral
    kin_reactions = 'a + b = mineral'
    log10_keq = '-6'
    specific_reactive_surface_area = '1.0'
    kinetic_rate_constant = '1.0e-8'
    activation_energy = '1.5e4'
    gas_constant = 8.314
    reference_temperature = '298.15'
    system_temperature = '298.15'
  [../]
[]

[Kernels]
  [./a_ie]
    type = PrimaryTimeDerivative
    variable = a
  [../]
  [./a_pd]
    type = PrimaryDiffusion
    variable = a
  [../]
  [./b_ie]
    type = PrimaryTimeDerivative
    variable = b
  [../]
  [./b_pd]
    type = PrimaryDiffusion
    variable = b
  [../]
[]

[BCs]
  [./a_left]
    type = DirichletBC
    variable = a
    preset = false
    boundary = left
    value = 1.0e-2
  [../]
  [./a_right]
    type = DirichletBC
    variable = a
    preset = false
    boundary = right
    value = 0
  [../]
  [./b_left]
    type = DirichletBC
    variable = b
    preset = false
    boundary = left
    value = 0
  [../]
  [./b_right]
    type = DirichletBC
    variable = b
    preset = false
    boundary = right
    value = 1.0e-2
  [../]
[]

[Materials]
  [./porous]
    type = GenericConstantMaterial
    prop_names = 'diffusivity conductivity porosity'
    prop_values = '5e-4 4e-3 0.4'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  end_time = 50
  dt = 5
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Outputs]
  file_base = 2species_out
  exodus = true
  perf_graph = true
  print_linear_residuals = true
[]
