#
# Simple power law creep example (without instantaneous plasticity)
# NOTE: this test is not replicated in a direct tensor mechanics form because the functioning
#   of the individual TM power law creep and TM isotropic plasticity is already tested in the
#   tensor mechanics module.
#
# The mesh is a 1x1x1 cube with a constant pressure of 10 MPa on the top face.
#   Symmetry boundary conditions on three planes provide a uniaxial stress
#   field. The temperature is held constant at 1000. The yield stress is set
#   at twice the load pressure, thus there is no plasticity.  The solution is
#   advanced through ten time steps of 0.1 for a total time of 1.
#
#   The total strain at time 1 can be computed as:
#
#    e_tot = e_elas + e_creep
#
#           = P/E   + A * sigma**n * exp(-Q/(RT)) * t**m * dt
#
#              where P = pressure load
#                    E = Young's modulus
#                    A = material parameter
#                    sigma = stress
#                    n = power law exponent
#                    Q = activation energy
#               R = gas constant
#                    T = temperature
#                    t = time
#                    m = time hardening exponent
#                    dt = problem time
#
# For this test, the analytical solutuon is:
#
#   e_tot = (10e6/2e11) + 1e-15 * (10e6)**4 * exp(-3e5/(8.3143*1000) * t**0 * 1
#         = 5e-5        + 2.136031e-3
#         = 2.186031e-3
#
#
#  For either linear (formulation = linear) or nonlinear (formulation = nonlinear3d)
#    kinematics, PLC_LSH gets:
#
#  e_elas = 5e-5
#  e_creep = 2.13600e-3
#  e_tot = 2.18600e-3
#
#
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[GlobalParams]
  displacements = 'x_disp y_disp z_disp'
[]

[Variables]
  [./x_disp]
    order = FIRST
    family = LAGRANGE
  [../]

  [./y_disp]
    order = FIRST
    family = LAGRANGE
  [../]

  [./z_disp]
    order = FIRST
    family = LAGRANGE
  [../]

  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1000.0
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

[Functions]
  [./top_pull]
    type = PiecewiseLinear

    x = '0 1'
    y = '1 1'
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = x_disp
    disp_y = y_disp
    disp_z = z_disp
  [../]
[]

[Kernels]
  [./solid_x_ie]
    type = SolidMechImplicitEuler
    variable = x_disp
  [../]

  [./solid_y_ie]
    type = SolidMechImplicitEuler
    variable = y_disp
  [../]

  [./solid_z_ie]
    type = SolidMechImplicitEuler
    variable = z_disp
  [../]

  [./heat]
    type = HeatConduction
    variable = temp
  [../]

  [./heat_ie]
    type = HeatConductionTimeDerivative
    variable = temp
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
    type = Pressure
    variable = y_disp
    component = 1
    boundary = top
    factor = -10.0e6
    function = top_pull
  [../]
  [./u_bottom_fix]
    type = DirichletBC
    variable = y_disp
    boundary = bottom
    value = 0.0
  [../]
  [./u_yz_fix]
    type = DirichletBC
    variable = x_disp
    boundary = left
    value = 0.0
  [../]
  [./u_xy_fix]
    type = DirichletBC
    variable = z_disp
    boundary = back
    value = 0.0
  [../]
  [./temp_fix]
    type = DirichletBC
    variable = temp
    boundary = 'top bottom'
    value = 1000.0
  [../]
[]

[Materials]

  [./fred]
    type = SolidModel
    block = 0
    youngs_modulus = 2.e11
    poissons_ratio = .3
    disp_x = x_disp
    disp_y = y_disp
    disp_z = z_disp
    temp = temp
    formulation = nonlinear3d
    constitutive_model = creep
  [../]
  [./creep]
    type = PowerLawCreepModel
    block = 0
    coefficient = 1.0e-15
    n_exponent = 4
    m_exponent = 0
    activation_energy = 3.0e5
    relative_tolerance = 1.e-5
    max_its = 100
    temp = temp
    output_iteration_info = false
  [../]

  [./thermal]
    type = HeatConductionMaterial
    block = 0
    specific_heat = 1.0
    thermal_conductivity = 100.
  [../]

  [./density]
    type = Density
    block = 0
    density = 1.0
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  petsc_options = '-snes_ksp'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'


  line_search = 'none'


  l_max_its = 10
  nl_max_its = 10
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  l_tol = 1e-5
  start_time = 0.0
  end_time = 1.0
  num_steps = 10
  dt = 0.1
[]

[Outputs]
  file_base = creep_only_combined_class_sm_out
  exodus = true
  csv = true
[]
