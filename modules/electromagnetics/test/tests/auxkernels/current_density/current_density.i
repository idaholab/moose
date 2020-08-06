# This test creates a current density field in graphite running from the top left
# corner of the domain (powered with a potential of 1 V) into the bottom right
# corner (a slice has been taken from this corner to provide a grounded surface).
# Current flow should proceed from the powered surfaces to the grounded surface.

[Mesh]
  [box]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 50
    elem_type = TRI6
  []
  [delete_corner]
    type = PlaneDeletionGenerator
    input = box
    point = '0.9 0.1 0'
    normal = '1 -1 0'
    new_boundary = 'corner'
  []
[]

[Variables]
  [potential]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxVariables]
  [J]
    family = NEDELEC_ONE
    order = FIRST
  []
[]

[Kernels]
  [poisson]
    type = Diffusion
    variable = potential
  []
[]

[BCs]
  [driven]
    type = DirichletBC
    variable = potential
    value = 1
    boundary = 'top left'
  []
  [grounded]
    type = DirichletBC
    variable = potential
    value = 0
    boundary = 'corner'
  []
[]

[AuxKernels]
  [current_density]
    type = ADCurrentDensity
    variable = J
    potential = potential
  []
[]

[Materials]
  [conductivity]
    type = ADGenericConstantMaterial
    prop_names = 'electrical_conductivity'
    prop_values = 3.33e2 # electrical conductivity for graphite at 293.15 K
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
  print_linear_converged_reason = false
  print_nonlinear_converged_reason = false
[]
