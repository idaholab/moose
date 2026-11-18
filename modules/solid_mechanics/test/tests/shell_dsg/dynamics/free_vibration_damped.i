# Free vibration of a cantilever plate with mass-proportional damping and the
# HHT shift of the damping term, DSG shell elements.
#
# The plate of free_vibration.i (1.0 m by 1.5 m, E = 2.1e6, nu = 0.3,
# thickness 0.1, density 1, clamped along its bottom edge) is set vibrating by
# the same force pulse. The inertia kernels carry the Rayleigh damping
# coefficient eta = 0.1 on the mass and the HHT parameter alpha = -0.1, which
# weights the damping force between the current and the previous velocity.
# The tip displacement history decays and is recorded.

[Mesh]
  [plate]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 1.5
    elem_type = QUAD4
  []
  [orientation]
    type = TransformGenerator
    input = plate
    transform = ROTATE
    vector_value = '0 0 0'
  []
  [tip_corners]
    type = ExtraNodesetGenerator
    input = orientation
    new_boundary = tip_corners
    coord = '0.0 1.5 0.0; 1.0 1.5 0.0'
  []
[]

[Variables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
  [disp_z]
    order = FIRST
    family = LAGRANGE
  []
  [rot_x]
    order = FIRST
    family = LAGRANGE
  []
  [rot_y]
    order = FIRST
    family = LAGRANGE
  []
  [rot_z]
    order = FIRST
    family = LAGRANGE
  []
[]

[BCs]
  [clamp_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  []
  [clamp_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [clamp_disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []
  [clamp_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = bottom
    value = 0.0
  []
  [clamp_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = bottom
    value = 0.0
  []
  [clamp_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = bottom
    value = 0.0
  []
[]

[Functions]
  [force_pulse]
    type = PiecewiseLinear
    x = '0.0 0.01 0.15 10.0'
    y = '0.0 0.01 0.0 0.0'
  []
[]

[NodalKernels]
  [tip_force]
    type = UserForcingFunctorNodalKernel
    variable = disp_z
    boundary = tip_corners
    functor = force_pulse
  []
[]

[Kernels]
  [solid_disp_x]
    type = StressDivergenceDSGShell
    component = 0
    variable = disp_x
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_disp_y]
    type = StressDivergenceDSGShell
    component = 1
    variable = disp_y
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_disp_z]
    type = StressDivergenceDSGShell
    component = 2
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_rot_x]
    type = StressDivergenceDSGShell
    component = 3
    variable = rot_x
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_rot_y]
    type = StressDivergenceDSGShell
    component = 4
    variable = rot_y
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_rot_z]
    type = StressDivergenceDSGShell
    component = 5
    variable = rot_z
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [inertia_disp_x]
    type = InertialForceDSGShell
    eta = 0.1
    alpha = -0.1
    component = 0
    variable = disp_x
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [inertia_disp_y]
    type = InertialForceDSGShell
    eta = 0.1
    alpha = -0.1
    component = 1
    variable = disp_y
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [inertia_disp_z]
    type = InertialForceDSGShell
    eta = 0.1
    alpha = -0.1
    component = 2
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [inertia_rot_x]
    type = InertialForceDSGShell
    eta = 0.1
    alpha = -0.1
    component = 3
    variable = rot_x
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [inertia_rot_y]
    type = InertialForceDSGShell
    eta = 0.1
    alpha = -0.1
    component = 4
    variable = rot_y
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [inertia_rot_z]
    type = InertialForceDSGShell
    eta = 0.1
    alpha = -0.1
    component = 5
    variable = rot_z
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
[]

[Materials]
  [strain]
    type = ComputeDSGShellStrain
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [section]
    type = ComputeDSGShellSectionStiffness
    youngs_modulus = 2.1e6
    poissons_ratio = 0.3
    thickness = 0.1
  []
  [resultants]
    type = ComputeDSGShellResultants
  []
  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '1.0'
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
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  dt = 0.005
  dtmin = 0.005
  end_time = 0.5
  timestep_tolerance = 2e-13
  [TimeIntegrator]
    type = NewmarkBeta
    beta = 0.25
    gamma = 0.5
  []
[]

[Postprocessors]
  [disp_z_tip]
    type = PointValue
    point = '1.0 1.5 0.0'
    variable = disp_z
  []
  [rot_x_tip]
    type = PointValue
    point = '0.0 1.5 0.0'
    variable = rot_x
  []
[]

[Outputs]
  csv = true
[]
