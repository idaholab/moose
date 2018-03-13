#This tests the J-Integral evaluation capability.
#This is a 3d extrusion of a 2d plane strain model with 2 elements
#through the thickness, and calculates the J-Integrals using options
#to treat it as 3d.
#The analytic solution for J1 is 2.434.  This model
#converges to that solution with a refined mesh.
#Reference: National Agency for Finite Element Methods and Standards (U.K.):
#Test 1.1 from NAFEMS publication "Test Cases in Linear Elastic Fracture
#Mechanics" R0020.

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[Mesh]
  file = crack3d.e
[]

[AuxVariables]
  [./SED]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./rampConstant]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = -1e2
  [../]
[]

[DomainIntegral]
  integrals = JIntegral
  crack_front_points = '0 -10 .5
                        0 -10 0
                        0 -10 -.5'
  crack_direction_method = CrackDirectionVector
  crack_direction_vector = '1 0 0'
  radius_inner = '4.0 5.5'
  radius_outer = '5.5 7.0'
  output_q = false
  incremental = true
[]

[Modules/TensorMechanics/Master]
  [./master]
    strain = FINITE
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
  [../]
[]

[AuxKernels]
  [./SED]
    type = MaterialRealAux
    variable = SED
    property = strain_energy_density
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./crack_y]
    type = DirichletBC
    variable = disp_y
    boundary = 100
    value = 0.0
  [../]
  [./no_z]
    type = DirichletBC
    variable = disp_z
    boundary = 500
    value = 0.0
  [../]
  [./no_z2]
    type = DirichletBC
    variable = disp_z
    boundary = 510
    value = 0.0
  [../]

  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 700
    value = 0.0
  [../]

  [./Pressure]
    [./Side1]
      boundary = 400
      function = rampConstant
    [../]
  [../]

[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 207000
    poissons_ratio = 0.3
  [../]
  [./elastic_stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]


[Executioner]
  type = Transient

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 20
  nl_abs_tol = 1e-5
  l_tol = 1e-2

  start_time = 0.0
  dt = 1

  end_time = 1
  num_steps = 1
[]

[Outputs]
  file_base = j_integral_3d_points_out
  exodus = true
  csv = true
[]
