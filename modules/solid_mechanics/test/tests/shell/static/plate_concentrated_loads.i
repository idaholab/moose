# A simply supported plate with a length of 9m and width of 1m is loaded by two equal concentrated loads (F=10000 N/m)
# The concentrated loads are symmetrically applied at x=3 and x=6

# Analytical solution: maximum diplacement at the center= 6.469e-3
# Numerical model: maximum diplacement at the center=6.436e-3

# Analytical solution: maximum bending moment (m22) at the center =30000
# Numerical model: maximum bending moment (m22) at the center =30000

# Analytical solution: out of plane shear force (q13) for 0<x<3 =10000
# Numerical model: out of plane shear force (q13) for 0<x<3 =10000

# Analytical solution: out of plane shear force (q13) for 3<x<6 =0
# Numerical model: out of plane shear force (q13) at for 3<x<6 =0

# Analytical solution: out of plane shear force (q13) for 6<x<9 =-10000
# Numerical model: out of plane shear force (q13) for 6<x<9 =-10000

[Mesh]
  [gmg]
    type = FileMeshGenerator
    file = Plate_Concentrated_Loads.msh
  []

  [p1]
    type = BoundingBoxNodeSetGenerator
    input = gmg
    bottom_left = '2.99 0.0 -0.1'
    top_right = '3.1 0.0 1.1'
    new_boundary = 100
  []

  [p2]
    type = BoundingBoxNodeSetGenerator
    input = p1
    bottom_left = '5.99 0.0 -0.1'
    top_right = '6.1 0.0 1.1'
    new_boundary = 101
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
[]

[BCs]

  [simply_support_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'right left'
    value = 0.0
  []
  [simply_support_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'right left'
    value = 0.0
  []
[]

[NodalKernels]
  [force_p1]
    type = ConstantRate
    variable = disp_y
    boundary = 100
    rate = -2500 # applied to the four nodes at x=3
  []

  [force_p2]
    type = ConstantRate
    variable = disp_y
    boundary = 101
    rate = -2500 # applied to the four nodes at x=6
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
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = ' lu       mumps'
  line_search = 'none'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  dt = 1
  dtmin = 1
  end_time = 1.
[]

[Kernels]
  [solid_disp_x]
    type = ADStressDivergenceShell
    component = 0
    variable = disp_x
    through_thickness_order = SECOND
  []
  [solid_disp_y]
    type = ADStressDivergenceShell
    component = 1
    variable = disp_y
    through_thickness_order = SECOND
  []
  [solid_disp_z]
    type = ADStressDivergenceShell
    component = 2
    variable = disp_z
    through_thickness_order = SECOND
  []
  [solid_rot_x]
    type = ADStressDivergenceShell
    component = 3
    variable = rot_x
    through_thickness_order = SECOND
  []
  [solid_rot_y]
    type = ADStressDivergenceShell
    component = 4
    variable = rot_y
    through_thickness_order = SECOND
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensorShell
    youngs_modulus = 200e9
    poissons_ratio = 0.0
    through_thickness_order = SECOND
  []
  [strain]
    type = ADComputeIncrementalShellStrain
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    thickness = 0.133887
    through_thickness_order = SECOND
  []
  [stress]
    type = ADComputeShellStress
    through_thickness_order = SECOND
  []
[]

[AuxVariables]

  [moment_22]
    order = CONSTANT
    family = MONOMIAL
  []
  [shear_13]
    order = CONSTANT
    family = MONOMIAL
  []
  [first_axis_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [first_axis_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [first_axis_z]
    order = CONSTANT
    family = MONOMIAL
  []
  [second_axis_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [second_axis_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [second_axis_z]
    order = CONSTANT
    family = MONOMIAL
  []
  [normal_axis_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [normal_axis_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [normal_axis_z]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]

  [moment_22]
    type = ShellResultantsAux
    variable = moment_22
    stress_resultant = bending_moment_1
    thickness = 0.133887
    through_thickness_order = SECOND
    execute_on = TIMESTEP_END
  []

  [shear_13]
    type = ShellResultantsAux
    variable = shear_13
    stress_resultant = shear_force_02
    thickness = 0.133887
    through_thickness_order = SECOND
    execute_on = TIMESTEP_END
  []
  [first_axis_x]
    type = ShellLocalCoordinatesAux
    variable = first_axis_x
    property = first_local_vector
    component = 0
  []
  [first_axis_y]
    type = ShellLocalCoordinatesAux
    variable = first_axis_y
    property = first_local_vector
    component = 1
  []
  [first_axis_z]
    type = ShellLocalCoordinatesAux
    variable = first_axis_z
    property = first_local_vector
    component = 2
  []

  [second_axis_x]
    type = ShellLocalCoordinatesAux
    variable = second_axis_x
    property = second_local_vector
    component = 0
  []
  [second_axis_y]
    type = ShellLocalCoordinatesAux
    variable = second_axis_y
    property = second_local_vector
    component = 1
  []
  [second_axis_z]
    type = ShellLocalCoordinatesAux
    variable = second_axis_z
    property = second_local_vector
    component = 2
  []

  [normal_axis_x]
    type = ShellLocalCoordinatesAux
    variable = normal_axis_x
    property = normal_local_vector
    component = 0
  []
  [normal_axis_y]
    type = ShellLocalCoordinatesAux
    variable = normal_axis_y
    property = normal_local_vector
    component = 1
  []
  [normal_axis_z]
    type = ShellLocalCoordinatesAux
    variable = normal_axis_z
    property = normal_local_vector
    component = 2
  []
[]

[Outputs]
  exodus = true
[]
