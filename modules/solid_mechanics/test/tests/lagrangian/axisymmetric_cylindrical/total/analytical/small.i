# This test aims to verify the implementation by comparing the numerical solution
# to the analytical solution. The Lame solution for a hollow cylinder is used in
# this test. Given inner pressure p = 100, outer pressure q = 200, inner radius
# a = 2, outer radius b = 4, the stress and displacements at the middle of the cylinder
# (r = 3, z = 0) should be
# sigma_rr = -174.074
# sigma_tt = -292.593
# u_r = -0.65972

# The numerical approximation yields
# sigma_rr = -172.051
# sigma_tt = -294.613
# u_r = -0.65964

[GlobalParams]
  displacements = 'disp_r disp_z'
  large_kinematics = false
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 2
    xmax = 4
    ymin = 0
    ymax = 10
    nx = 20
    ny = 100
  []
[]

[Problem]
  coord_type = RZ
[]

[Variables]
  [disp_r]
  []
  [disp_z]
  []
[]

[Kernels]
  [sdr]
    type = TotalLagrangianStressDivergenceAxisymmetricCylindrical
    variable = disp_r
    component = 0
  []
  [sdz]
    type = TotalLagrangianStressDivergenceAxisymmetricCylindrical
    variable = disp_z
    component = 1
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = disp_z
    boundary = 'top bottom'
    value = 0.0
    preset = false
  []
  [inner]
    type = NeumannBC
    variable = disp_r
    boundary = left
    value = 100
  []
  [outer]
    type = NeumannBC
    variable = disp_r
    boundary = right
    value = -200
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000.0
    poissons_ratio = 0.25
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
    output_properties = 'cauchy_stress'
    outputs = 'exodus'
  []
  [compute_strain]
    type = ComputeLagrangianStrainAxisymmetricCylindrical
  []
[]

[Postprocessors]
  [u_r]
    type = PointValue
    variable = disp_r
    point = '3 0 0'
  []
  [sigma_rr]
    type = PointValue
    variable = cauchy_stress_00
    point = '3 0 0'
  []
  [sigma_tt]
    type = PointValue
    variable = cauchy_stress_22
    point = '3 0 0'
  []
[]

[Executioner]
  type = Transient

  solve_type = NEWTON

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10

  num_steps = 1
[]

[Outputs]
  exodus = true
[]
