# Simply supported square plate under a uniform load, DSG shell elements.
#
# One quarter of a 50 m x 50 m x 1 m plate is modeled with 25 x 25 four-node
# elements, E = 1e9 and nu = 0.3. The load is applied as a nodal force of -10 N
# on every node, which corresponds to a pressure q = -10.816 N/m^2 on the full
# plate. The symmetry edges carry the rotation about their own direction, the
# supported edges carry the transverse displacement, and the in-plane
# displacements are fixed on all edges.
#
# The thin plate solution of Timoshenko and Woinowsky-Krieger (1959) for the
# center deflection is the Navier series
#   w = 16 q a^4 / (D pi^6) sum_m sum_n (-1)^((m+n-2)/2) / (m n (m^2 + n^2)^2)
# over odd m and n, with a = 50 m and D = E t^3 / (12 (1 - nu^2)), which
# converges to 2.998536e-03 m. The center of the full plate is the point (0, 0).

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 25
    ny = 25
    xmin = 0.0
    xmax = 25.0
    ymin = 0.0
    ymax = 25.0
  []
  [allnodes]
    type = BoundingBoxNodeSetGenerator
    input = gmg
    bottom_left = '0.0 0.0 0.0'
    top_right = '25.0 25.0 0.0'
    new_boundary = 101
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [rot_x]
  []
  [rot_y]
  []
  [rot_z]
  []
[]

[BCs]
  [symm_left_rot]
    type = DirichletBC
    variable = rot_y
    boundary = left
    value = 0.0
  []
  [symm_bottom_rot]
    type = DirichletBC
    variable = rot_x
    boundary = bottom
    value = 0.0
  []
  [simply_support_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'right top bottom left'
    value = 0.0
  []
  [simply_support_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'right top bottom left'
    value = 0.0
  []
  [simply_support_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'right top'
    value = 0.0
  []
[]

[NodalKernels]
  [pressure_as_nodal_forces]
    type = ConstantRate
    variable = disp_z
    boundary = 101
    rate = -10.0
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
[]

[Materials]
  [strain]
    type = ComputeDSGShellStrain
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [section]
    type = ComputeDSGShellSectionStiffness
    youngs_modulus = 1e9
    poissons_ratio = 0.3
    thickness = 1.0
  []
  [resultants]
    type = ComputeDSGShellResultants
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
  nl_abs_tol = 1e-8
  dt = 1.0
  dtmin = 1.0
  end_time = 1.0
[]

[Postprocessors]
  [disp_z_center]
    type = PointValue
    point = '0.0 0.0 0.0'
    variable = disp_z
  []
[]

[Outputs]
  csv = true
[]
