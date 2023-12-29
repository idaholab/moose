#
# 1x1x1 unit cube with constant displacement on top face
#
# This problem was taken from "Finite element three-dimensional elastic-plastic
#    creep analysis" by A. Levy, Eng. Struct., 1981, Vol. 3, January, pp. 9-16.
#
# The problem is a one-dimensional creep analysis.  The top face is displaced 0.01
#    units and held there.  The stress relaxes in time according to the creep law.
#
# The analytic solution to this problem is (contrary to what is shown in the paper):
#
#                 /      (E*ef)^3      \^(1/3)
#    stress_yy = |---------------------|
#                \ 3*a*E^4*ef^3*t + 1 /
#
#    where E  = 2.0e11  (Young's modulus)
#          a  = 3e-26  (creep coefficient)
#          ef = 0.01   (displacement)
#          t  = 2160.0    (time)
#
#    such that the analytical solution is computed to be 2.9518e3 Pa
#
# Averaged over the single element block, MOOSE calculates the stress in the yy direction to be
#     to be 3.046e3 Pa, which is a 3.2% error from the analytical solution.
#

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[Functions]
  [./dts]
    type = PiecewiseLinear
    y = '1e-2 1e-1 1e0 1e1 1e2'
    x = '0    7e-1 7e0 7e1 1e2'
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    incremental = true
    add_variables = true
    generate_output = 'stress_yy creep_strain_xx creep_strain_yy creep_strain_zz elastic_strain_yy'
  [../]
[]

[BCs]
  [./u_top_pull]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = 0.01
  [../]
  [./u_bottom_fix]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./u_yz_fix]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./u_xy_fix]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.0e11
    poissons_ratio = 0.3
  [../]
  [./radial_return_stress]
    type = ComputeMultipleInelasticStress
    tangent_operator = elastic
    inelastic_models = 'power_law_creep'
  [../]
  [./power_law_creep]
    type = PowerLawCreepStressUpdate
    coefficient = 3.0e-26
    n_exponent = 4
    activation_energy = 0.0
    relative_tolerance = 1e-14
    absolute_tolerance = 1e-14
  [../]
[]

[Postprocessors]
  [./stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-8
  l_tol = 1e-5
  start_time = 0.0
  end_time = 2160

  [./TimeStepper]
    type = FunctionDT
    function = dts
  [../]
[]

[Outputs]
  exodus = true
[]
