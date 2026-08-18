# Lee's frame traced by per-timestep arc-length continuation.
#
# A column and a beam of equal length meet at a right angle, hinged at the
# column base and at the far end of the beam, with a dead load pressing down
# on the beam near the joint. The frame resists by sway of the column, and the
# load-deflection curve is the canonical post-buckling benchmark: it rises to
# a limit point, falls on a steep descending branch, and snaps back before
# rising again. The companion input runs the prescribed loading to its
# failure:
#   lee_frame_load_control.i  (Newton under load control: nothing to converge
#                              to above the limit point)
#
# Every time step advances the trace by one continuation increment of radius
# step_size and commits the state it reaches. On the elastic rise the load
# factor tracks the time; at the limit point it parts from it, falling with
# the descending branch and turning back up after the snap back, while the
# time keeps counting arc steps. The path is elastic, so nothing needs
# committing between increments -- what the per-step trace shows here is the
# load factor running free of the time on a curve that neither prescribed
# loading can follow.
#
# Geometry: legs 120 long and 2 thick, meshed as an L-shaped strip of 0.5
# elements, four through the thickness. The load is a traction on the stiff
# patch at 24 from the joint. lambda = 1 corresponds to a resultant of 5.

leg = 120
thick = 2
nx = 240

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${nx}
    ny = ${nx}
    xmax = ${leg}
    ymax = ${leg}
    elem_type = QUAD4
  []
  [interior]
    type = SubdomainBoundingBoxGenerator
    input = square
    block_id = 9
    bottom_left = '${thick} 0 0'
    top_right = '${leg} ${fparse leg - thick} 0'
  []
  [carve]
    type = BlockDeletionGenerator
    input = interior
    block = 9
  []
  # near-rigid patches at the supports and under the load spread the point
  # reactions over the section, so the hinge is a rotation of the patch and
  # not a finite-strain distortion of the two elements next to a point pin
  [base_patch]
    type = SubdomainBoundingBoxGenerator
    input = carve
    block_id = 2
    bottom_left = '0 0 0'
    top_right = '${thick} ${thick} 0'
  []
  [end_patch]
    type = SubdomainBoundingBoxGenerator
    input = base_patch
    block_id = 2
    bottom_left = '${fparse leg - thick} ${fparse leg - thick} 0'
    top_right = '${leg} ${leg} 0'
  []
  [load_patch]
    type = SubdomainBoundingBoxGenerator
    input = end_patch
    block_id = 2
    bottom_left = '${fparse thick + 23} ${fparse leg - thick} 0'
    top_right = '${fparse thick + 25} ${leg} 0'
  []
  # hinges: a single mid-thickness node at each support leaves the section
  # free to rotate about it
  [column_hinge]
    type = ExtraNodesetGenerator
    input = load_patch
    new_boundary = column_hinge
    coord = '${fparse thick / 2} 0 0'
  []
  [beam_hinge]
    type = ExtraNodesetGenerator
    input = column_hinge
    new_boundary = beam_hinge
    coord = '${leg} ${fparse leg - thick / 2} 0'
  []
  # the top face of the load patch, so the load enters as a traction spread
  # over the stiff patch rather than a point source
  [load_face]
    type = ParsedGenerateSideset
    input = beam_hinge
    combinatorial_geometry = 'y > ${fparse leg - 1e-9} & x > ${fparse thick + 23 - 1e-9} & x < ${fparse thick + 25 + 1e-9}'
    normal = '0 1 0'
    new_sideset_name = load_face
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    decomposition_method = TaylorExpansion
    use_automatic_differentiation = true
    add_variables = true
  []
[]

[BCs]
  # the continuation load: a dead traction on the load patch, resultant 5
  # downward at lambda = 1, at 24 from the joint
  [beam_load]
    type = NeumannBC
    variable = disp_y
    boundary = load_face
    value = ${fparse -5 / 2}
    vector_tags = 'arc_length_load'
  []
  [column_hinge_x]
    type = DirichletBC
    variable = disp_x
    boundary = column_hinge
    value = 0
  []
  [column_hinge_y]
    type = DirichletBC
    variable = disp_y
    boundary = column_hinge
    value = 0
  []
  [beam_hinge_x]
    type = DirichletBC
    variable = disp_x
    boundary = beam_hinge
    value = 0
  []
  [beam_hinge_y]
    type = DirichletBC
    variable = disp_y
    boundary = beam_hinge
    value = 0
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 71240
    poissons_ratio = 0.3
    block = 0
  []
  [elasticity_patches]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 7.124e6
    poissons_ratio = 0.3
    block = 2
  []
  [stress]
    type = ADComputeFiniteStrainElasticStress
  []
[]

[Problem]
  type = ArcLengthProblem
  # a radius that resolves the turns of the path, kept through every cutback:
  # a retry shrinks the load span of the step, not the radius. On the smooth
  # rise the radius is slack and the load factor tracks the time; past the
  # limit point the radius is what carries the trace
  step_size = 10
  psi_squared = 0
  correction_type = normal
[]

[Postprocessors]
  [lambda]
    type = ArcLengthLoadParameter
  []
  [P]
    type = ScalePostprocessor
    value = lambda
    scaling_factor = 5
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [load_point_v]
    type = PointValue
    variable = disp_y
    point = '${fparse thick + 24} ${leg} 0'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [joint_sway]
    type = PointValue
    variable = disp_x
    point = '${fparse thick / 2} ${fparse leg - thick / 2} 0'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
[]

[VectorPostprocessors]
  [path]
    type = ArcLengthHistory
    postprocessors = 'load_point_v joint_sway'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = none
  automatic_scaling = true
  # pseudo time: counts arc steps; the trace runs past the limit point, down
  # the descending branch, through the snap back and the negative trough, and
  # onto the recovery
  dt = 0.1
  end_time = 80
  dtmin = 1e-12
  nl_max_its = 50
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
  exodus = true
[]
