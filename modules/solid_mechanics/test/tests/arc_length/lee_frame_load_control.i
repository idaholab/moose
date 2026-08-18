# The frame of lee_frame_arclength.i under plain load control: the same point
# load is ramped directly with time, load factor = t, and every step is an
# ordinary Newton solve. Above the limit point there is no equilibrium at a
# higher load, so the step at the peak has nothing to converge to: Newton
# diverges, the TimeStepper cuts back to dtmin, and the run aborts. The
# arc-length run traces on where this one stops.
#
# Run from this directory:
#   ../../../../combined/combined-opt -i lee_frame_load_control.i

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
  # the same load, ramped directly: load factor = t
  [beam_load]
    type = FunctionNeumannBC
    variable = disp_y
    boundary = load_face
    function = '${fparse -5 / 2} * t'
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

[Postprocessors]
  [P]
    type = FunctionValuePostprocessor
    function = '5 * t'
  []
  [load_point_v]
    type = PointValue
    variable = disp_y
    point = '${fparse thick + 24} ${leg} 0'
  []
  [joint_sway]
    type = PointValue
    variable = disp_x
    point = '${fparse thick / 2} ${fparse leg - thick / 2} 0'
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
  dt = 0.25
  end_time = 20
  dtmin = 0.01
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
