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
#                /       (E*ef)^3     \^(1/3)
#    stress_yy = |--------------------|
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
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./dts]
    type = PiecewiseLinear
    y = '1e-2 1e-1 1e0 1e1 1e2'
    x = '0    7e-1 7e0 7e1 1e2'
  [../]
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./creep_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./creep_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./creep_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./elastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[AuxKernels]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
  [./creep_strain_xx]
    type = MaterialTensorAux
    tensor = creep_strain
    variable = creep_strain_xx
    index = 0
  [../]
  [./creep_strain_yy]
    type = MaterialTensorAux
    tensor = creep_strain
    variable = creep_strain_yy
    index = 1
  [../]
  [./creep_strain_zz]
    type = MaterialTensorAux
    tensor = creep_strain
    variable = creep_strain_zz
    index = 2
  [../]
  [./elastic_strain_yy]
    type = MaterialTensorAux
    tensor = elastic_strain
    variable = elastic_strain_yy
    index = 1
  [../]
[]

[BCs]
  [./u_top_pull]
    type = PresetBC
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
  [./creep]
    type = PowerLawCreep
    block = 0
    youngs_modulus = 2.0e11
    poissons_ratio = 0.3
    coefficient = 3.0e-26
    n_exponent = 4
    activation_energy = 0.0
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    output_iteration_info = false
    relative_tolerance = 1e-14
    absolute_tolerance = 1e-14
    formulation = Nonlinear3D
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
#  petsc_options = '-snes_mf_operator -ksp_monitor -snes_ksp_ew'

  #Preconditioned JFNK (default)
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
  file_base = combined_stress_relaxation_out
[]
