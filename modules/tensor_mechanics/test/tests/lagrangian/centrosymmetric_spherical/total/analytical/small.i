# This test aims to verify the implementation by comparing the numerical solution
# to the analytical solution. The analytical solution for a hollow sphere is used in
# this test. Given inner pressure p = 100, inner radius a = 2, outer radius b = 4,
# the stress and displacements at the middle of the cylinder
# (r = 3) should be
# sigma_rr = -19.57
# sigma_tt = sigma_pp = 31.22
# u_r = 0.08492

# The numerical approximation yields
# sigma_rr = -19.92
# sigma_tt = sigma_pp = 31.39
# u_r = 0.08492

[GlobalParams]
  displacements = 'disp_r'
  large_kinematics = false
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 2
    xmax = 4
    nx = 100
  []
[]

[Problem]
  coord_type = RSPHERICAL
[]

[Variables]
  [disp_r]
  []
[]

[Kernels]
  [sdr]
    type = TotalLagrangianStressDivergenceCentrosymmetricSpherical
    variable = disp_r
    component = 0
  []
[]

[BCs]
  [inner]
    type = NeumannBC
    variable = disp_r
    boundary = left
    value = 100
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
    type = ComputeLagrangianStrainCentrosymmetricSpherical
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
    variable = cauchy_stress_11
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
