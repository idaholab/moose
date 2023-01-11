# This problem is intended to exercise the Jacobian for coupled RZ
# problems.  Only two iterations should be needed.

[GlobalParams]
  temperature = temp
  volumetric_locking_correction = true
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = elastic_thermal_patch_rz_test.e
[]

[Functions]
  [./ur]
    type = ParsedFunction
    expression = '0'
  [../]
  [./uz]
    type = ParsedFunction
    expression = '0'
  [../]
  [./body]
    type = ParsedFunction
    expression = '-400/x'
  [../]
  [./temp]
    type = ParsedFunction
    expression = '117.56+100*t'
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]

  [./temp]
    initial_condition = 117.56
  [../]
[]

[Modules]
    [TensorMechanics]
        [Master]
            displacements = 'disp_x disp_y'
            [All]
                displacements = 'disp_x disp_y'
                add_variables = true
                strain = SMALL
                incremental = true
                eigenstrain_names = eigenstrain
                generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
            [../]
        [../]
    [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]

[BCs]
  [./ur]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 1
    function = ur
  [../]
  [./uz]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 2
    function = uz
  [../]

  [./temp]
    type = FunctionDirichletBC
    variable = temp
    boundary = 10
    function = temp
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.25
  [../]
  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain
  [../]
  [./stress]
    type = ComputeStrainIncrementBasedStress
  [../]

  [./heat]
    type = HeatConductionMaterial
    specific_heat = 0.116
    thermal_conductivity = 4.85e-4
  [../]

  [./density]
    type = Density
    density = 0.283
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  nl_abs_tol = 1e-9
  nl_rel_tol = 1e-12

  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  num_steps = 1
  end_time = 1.0
[]

[Outputs]
  file_base = elastic_thermal_jacobian_rz_smp_out
  [./exodus]
    type = Exodus
    execute_on = 'initial timestep_end nonlinear'
    nonlinear_residual_dt_divisor = 100
  [../]
[]
