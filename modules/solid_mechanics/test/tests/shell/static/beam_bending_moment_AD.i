# Test that models bending of a cantilever beam using shell elements

# A cantilever beam of length 10 m (in Y direction) and cross-section
# 1 m x 0.1 m is modeled using 4 shell elements placed along the length
# (Figure 6a from Dvorkin and Bathe, 1984). All displacements and
# X rotations are fixed on the bottom boundary. E = 2100000 and v = 0.0.
# A load of 0.5 N (in the Z direction) is applied at each node on the top
# boundary resulting in a total load of 1 N.

# The analytical solution for displacement at tip using small strain/rotations # is PL^3/3EI + PL/AG = 1.90485714 m
# The FEM solution using 4 shell elements is 1.875095 m with a relative error
# of 1.5%.

# Similarly, the analytical solution for slope at tip is PL^2/2EI = 0.285714286
# The FEM solution is 0.2857143 and the relative error is 5e-6%.

# The stress_yy for the four elements at z = -0.57735 * (t/2) (first qp below mid-surface of shell) are:
# 3031.089 Pa, 2165.064 Pa, 1299.038 Pa and 433.0127 Pa.
# Note the above values are the average stresses in each element.

# Analytically, stress_yy decreases linearly from y = 0 to y = 10 m.
# The maximum value of stress_yy at y = 0 is Mz/I = PL * 0.57735*(t/2)/I = 3464.1 Pa
# Therefore, the analytical value of stress at z = -0.57735 * (t/2) at the mid-point
# of the four elements are:
# 3031.0875 Pa, 2165.0625 Pa, 1299.0375 Pa ,433.0125 Pa

# The relative error in stress_yy is in the order of 5e-5%.

# The stress_yz at z = -0.57735 * (t/2) at all four elements from the simulation is 10 Pa.
# The analytical solution for the shear stress is: V/2/I *((t^2)/4 - z^2), where the shear force (V)
# is 1 N at any y along the length of the beam. Therefore, the analytical shear stress at
# z = -0.57735 * (t/2) is 10 Pa at any location along the length of the beam.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 4
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 10.0
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

[AuxVariables]
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yz]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [stress_yy]
    type = RankTwoAux
    variable = stress_yy
    rank_two_tensor = global_stress_t_points_0
    index_i = 1
    index_j = 1
  []
  [stress_yz]
    type = RankTwoAux
    variable = stress_yz
    rank_two_tensor = global_stress_t_points_0
    index_i = 1
    index_j = 2
  []
[]

[BCs]
  [fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0.0
  []
  [fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = 'bottom'
    value = 0.0
  []
  [fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = 'bottom'
    value = 0.0
  []
  [fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = 'bottom'
    value = 0.0
  []
  [fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = 'bottom'
    value = 0.0
  []
[]

[NodalKernels]
  [force_y2]
    type = ConstantRate
    variable = disp_z
    boundary = 'top'
    rate = 0.5
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
  nl_max_its = 2
  nl_rel_tol = 1e-10
  nl_abs_tol = 5e-4

  dt = 1
  dtmin = 1
  end_time = 1
[]

[Kernels]
  [solid_disp_x]
    type = ADStressDivergenceShell
    block = '0'
    component = 0
    variable = disp_x
    through_thickness_order = SECOND
  []
  [solid_disp_y]
    type = ADStressDivergenceShell
    block = '0'
    component = 1
    variable = disp_y
    through_thickness_order = SECOND
  []
  [solid_disp_z]
    type = ADStressDivergenceShell
    block = '0'
    component = 2
    variable = disp_z
    through_thickness_order = SECOND
  []
  [solid_rot_x]
    type = ADStressDivergenceShell
    block = '0'
    component = 3
    variable = rot_x
    through_thickness_order = SECOND
  []
  [solid_rot_y]
    type = ADStressDivergenceShell
    block = '0'
    component = 4
    variable = rot_y
    through_thickness_order = SECOND
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensorShell
    youngs_modulus = 2100000
    poissons_ratio = 0.0
    block = 0
    through_thickness_order = SECOND
  []
  [strain]
    type = ADComputeIncrementalShellStrain
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    thickness = 0.1
    through_thickness_order = SECOND
  []
  [stress]
    type = ADComputeShellStress
    block = 0
    through_thickness_order = SECOND
  []
[]

[Postprocessors]
  [disp_z_tip]
    type = PointValue
    point = '1.0 10.0 0.0'
    variable = disp_z
  []
  [rot_x_tip]
    type = PointValue
    point = '0.0 10.0 0.0'
    variable = rot_x
  []
  [stress_yy_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_yy
  []
  [stress_yy_el_1]
    type = ElementalVariableValue
    elementid = 1
    variable = stress_yy
  []
  [stress_yy_el_2]
    type = ElementalVariableValue
    elementid = 2
    variable = stress_yy
  []
  [stress_yy_el_3]
    type = ElementalVariableValue
    elementid = 3
    variable = stress_yy
  []
  [stress_yz_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_yz
  []
  [stress_yz_el_1]
    type = ElementalVariableValue
    elementid = 1
    variable = stress_yz
  []
  [stress_yz_el_2]
    type = ElementalVariableValue
    elementid = 2
    variable = stress_yz
  []
  [stress_yz_el_3]
    type = ElementalVariableValue
    elementid = 3
    variable = stress_yz
  []
[]

[Outputs]
  exodus = true
[]
