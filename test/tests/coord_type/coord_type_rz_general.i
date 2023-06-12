# Tests using different coordinate systems in different blocks:
#   block1: XYZ translated by (0,-1,0)
#   block2: RZ with origin=(0,0,0) and direction=(0,1,0)
#   block3: RZ with origin=(0,0,1) and direction=(1,0,0)
#   block4: RZ with origin=(-1,-2,-3) and direction=(1,1,0)
#
# A transient heat conduction equation is solved with uniform properties.
# The same power is applied to each block via a uniform heat flux boundary
# condition on the outer cylindrical surface (top surface for block1).
# Conservation is checked for each via post-processors.
# Blocks block2, block3, and block4 should have identical solutions.

rho = 1000.0
cp = 500.0
k = 15.0

length = 1.5
radius = 0.5

perimeter = ${fparse 2 * pi * radius}

nz = 10
nr = 5

power = 1e3
heat_flux = ${fparse power / (perimeter * length)}

[Mesh]
  # block1
  [genmesh1]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${nz}
    ny = ${nr}
    xmin = 0.0
    xmax = ${length}
    ymin = -1.0
    ymax = ${fparse -1.0 + radius}
    boundary_id_offset = 10
  []
  [renumberblock1]
    type = RenameBlockGenerator
    input = genmesh1
    old_block = 0
    new_block = 1
  []
  [renameblock1]
    type = RenameBlockGenerator
    input = renumberblock1
    old_block = 1
    new_block = block1
  []
  [renameboundary1]
    type = RenameBoundaryGenerator
    input = renameblock1
    old_boundary = '10 11 12 13'
    new_boundary = 'bottom1 right1 top1 left1'
  []

  # block2
  [genmesh2]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${nr}
    ny = ${nz}
    xmin = 0.0
    xmax = ${radius}
    ymin = 0
    ymax = ${length}
    boundary_id_offset = 20
  []
  [renumberblock2]
    type = RenameBlockGenerator
    input = genmesh2
    old_block = 0
    new_block = 2
  []
  [renameblock2]
    type = RenameBlockGenerator
    input = renumberblock2
    old_block = 2
    new_block = block2
  []
  [renameboundary2]
    type = RenameBoundaryGenerator
    input = renameblock2
    old_boundary = '20 21 22 23'
    new_boundary = 'bottom2 right2 top2 left2'
  []

  # block3
  [genmesh3]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${nz}
    ny = ${nr}
    xmin = 0.0
    xmax = ${length}
    ymin = 0
    ymax = ${radius}
    boundary_id_offset = 30
  []
  [translate3]
    type = TransformGenerator
    input = genmesh3
    transform = TRANSLATE
    vector_value = '0 0 1'
  []
  [renumberblock3]
    type = RenameBlockGenerator
    input = translate3
    old_block = 0
    new_block = 3
  []
  [renameblock3]
    type = RenameBlockGenerator
    input = renumberblock3
    old_block = 3
    new_block = block3
  []
  [renameboundary3]
    type = RenameBoundaryGenerator
    input = renameblock3
    old_boundary = '30 31 32 33'
    new_boundary = 'bottom3 right3 top3 left3'
  []

  # block4
  [genmesh4]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${nz}
    ny = ${nr}
    xmin = 0.0
    xmax = ${length}
    ymin = 0
    ymax = ${radius}
    boundary_id_offset = 40
  []
  [rotate4]
    type = TransformGenerator
    input = genmesh4
    transform = ROTATE
    vector_value = '45 0 0'
  []
  [translate4]
    type = TransformGenerator
    input = rotate4
    transform = TRANSLATE
    vector_value = '-1 -2 -3'
  []
  [renumberblock4]
    type = RenameBlockGenerator
    input = translate4
    old_block = 0
    new_block = 4
  []
  [renameblock4]
    type = RenameBlockGenerator
    input = renumberblock4
    old_block = 4
    new_block = block4
  []
  [renameboundary4]
    type = RenameBoundaryGenerator
    input = renameblock4
    old_boundary = '40 41 42 43'
    new_boundary = 'bottom4 right4 top4 left4'
  []

  [combiner]
    type = CombinerGenerator
    inputs = 'renameboundary1 renameboundary2 renameboundary3 renameboundary4'
  []

  coord_block = 'block1 block2 block3 block4'
  coord_type = 'XYZ RZ RZ RZ'

  rz_coord_blocks = 'block2 block3 block4'
  rz_coord_origins = '0 0 0
                      0 0 1
                      -1 -2 -3'
  rz_coord_directions = '0 1 0
                         1 0 0
                         1 1 0'
[]

[Variables]
  [T]
    family = LAGRANGE
    order = FIRST
  []
[]

[Functions]
  [T_ic_fn]
    type = ParsedFunction
    expression = 'x'
  []
  [theoretical_energy_added_fn]
    type = ParsedFunction
    expression = '${power} * t'
  []
[]

[ICs]
  [T_ic]
    type = FunctionIC
    variable = T
    function = T_ic_fn
  []
[]

[Kernels]
  [time_derivative]
    type = ADTimeDerivative
    variable = T
  []
  [heat_conduction]
    type = CoefDiffusion
    variable = T
    coef = ${fparse k / (rho * cp)}
  []
[]

[BCs]
  [heat_flux_bc]
    type = ADFunctionNeumannBC
    variable = T
    boundary = 'top1 right2 top3 top4'
    # The heat conduction equation has been divided by rho*cp
    function = '${fparse heat_flux / (rho * cp)}'
  []
[]

[Postprocessors]
  [theoretical_energy_change]
    type = FunctionValuePostprocessor
    function = theoretical_energy_added_fn
    execute_on = 'INITIAL TIMESTEP_END'
  []

  # block1 conservation
  [T_integral1]
    type = ElementIntegralVariablePostprocessor
    variable = T
    block = 'block1'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy1]
    type = ParsedPostprocessor
    pp_names = 'T_integral1'
    function = 'T_integral1 * ${rho} * ${cp} * ${perimeter}'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy_change1]
    type = ChangeOverTimePostprocessor
    postprocessor = energy1
    change_with_respect_to_initial = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy_change_error1]
    type = RelativeDifferencePostprocessor
    value1 = energy_change1
    value2 = theoretical_energy_change
    execute_on = 'INITIAL TIMESTEP_END'
  []

  # block2 conservation
  [T_integral2]
    type = ElementIntegralVariablePostprocessor
    variable = T
    block = 'block2'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy2]
    type = ParsedPostprocessor
    pp_names = 'T_integral2'
    function = 'T_integral2 * ${rho} * ${cp}'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy_change2]
    type = ChangeOverTimePostprocessor
    postprocessor = energy2
    change_with_respect_to_initial = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy_change_error2]
    type = RelativeDifferencePostprocessor
    value1 = energy_change2
    value2 = theoretical_energy_change
    execute_on = 'INITIAL TIMESTEP_END'
  []

  # block3 conservation
  [T_integral3]
    type = ElementIntegralVariablePostprocessor
    variable = T
    block = 'block3'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy3]
    type = ParsedPostprocessor
    pp_names = 'T_integral3'
    function = 'T_integral3 * ${rho} * ${cp}'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy_change3]
    type = ChangeOverTimePostprocessor
    postprocessor = energy3
    change_with_respect_to_initial = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy_change_error3]
    type = RelativeDifferencePostprocessor
    value1 = energy_change3
    value2 = theoretical_energy_change
    execute_on = 'INITIAL TIMESTEP_END'
  []

  # block4 conservation
  [T_integral4]
    type = ElementIntegralVariablePostprocessor
    variable = T
    block = 'block4'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy4]
    type = ParsedPostprocessor
    pp_names = 'T_integral4'
    function = 'T_integral4 * ${rho} * ${cp}'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy_change4]
    type = ChangeOverTimePostprocessor
    postprocessor = energy4
    change_with_respect_to_initial = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [energy_change_error4]
    type = RelativeDifferencePostprocessor
    value1 = energy_change4
    value2 = theoretical_energy_change
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  dt = 1.0
  num_steps = 10
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-10
[]

[Outputs]
  file_base = 'coord_type_rz_general'
  [console]
    type = Console
    show = 'energy_change_error1 energy_change_error2 energy_change_error3 energy_change_error4'
  []
  [exodus]
    type = Exodus
    show = 'T energy_change_error1 energy_change_error2 energy_change_error3 energy_change_error4'
  []
[]
