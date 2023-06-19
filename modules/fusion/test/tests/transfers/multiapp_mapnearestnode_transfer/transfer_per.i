[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 80
    xmax = 2
    ny = 80
    ymax = 2
  []

  [subdomain2]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.8 0.8 0'
    top_right = '1.2 1.2 0'
    block_id = 1
  []

  [interface1]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain2
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'boundary_interface_1'
  []

  [ed0]
    type = BlockDeletionGenerator
    input = interface1
    block = "1"
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff_u]
    type = MatDiffusion
    variable = u
    diffusivity = D
    block = 0
  []
  [source_u]
    type = BodyForce
    variable = u
    value = 1
    block = 0
  []
[]

[BCs]
#  [u]
#    type = VacuumBC
#    variable = u
#    boundary = 'left bottom right top'
#  []

  #active = 'interface1'
  [./Periodic]
    [./manual_x]
      variable = u
      primary = 'left'
      secondary = 'right'
      translation = '2 0 0'
    [../]
    [./manual_y]
      variable = u
      primary = 'bottom'
      secondary = 'top'
      translation = '0 2 0'
    [../]
  []

  [interface1]
    type = DirichletBC
    variable = u
    value = 10
    boundary = boundary_interface_1
  []
[]

[AuxVariables]
  [from_sub]
  []
  [from_sub_elemental_var]
  []
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
#  fixed_point_max_its = 2
#  fixed_point_rel_tol = 1e-6
  nl_rel_tol = 1e-6
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
#  print_linear_residuals = true
#  checkpoint = true
[]

[Materials]
  [mat0]
    type = GenericConstantMaterial
    prop_names = 'D'
    prop_values = '1'
    block = 0
  []
[]
