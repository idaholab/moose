# Simple reaction-diffusion example without using the action.
# In this example, two primary species a and b diffuse towards each other from
# opposite ends of a porous medium, reacting when they meet to form a mineral
# precipitate
# This simulation is identical to 2species.i, but explicitly includes the AuxVariables,
# AuxKernels, and Kernels that the action in 2species.i adds

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

[AuxVariables]
  [./mineral]
  [../]
[]

[AuxKernels]
  [./mineral_conc]
    type = KineticDisPreConcAux
    variable = mineral
    e_act = 1.5e4
    r_area = 1
    log_k = -6
    ref_kconst = 1e-8
    gas_const = 8.314
    ref_temp = 298.15
    sys_temp = 298.15
    sto_v = '1 1'
    v = 'a b'
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
  [./a_r]
    type = CoupledBEKinetic
    variable = a
    v = mineral
    weight = 1
  [../]
  [./b_r]
    type = CoupledBEKinetic
    variable = b
    v = mineral
    weight = 1
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
