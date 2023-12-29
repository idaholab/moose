[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  file = crack2d.e
[]

[AuxVariables]
  [./SED]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./tempfunc]
    type = ParsedFunction
    expression = 10.0*(2*x/504)
  [../]
  [./cte_func]
    type = PiecewiseLinear
    x = '-10 -6 -2 0 2 6 10'
    y = '1.484e-5 1.489e-5 1.494e-5 1.496e-5 1.498e-5 1.502e-5 1.505e-5'
  [../]
[]

[DomainIntegral]
  integrals = JIntegral
  boundary = 800
  crack_direction_method = CrackDirectionVector
  crack_direction_vector = '1 0 0'
  2d = true
  axis_2d = 2
  radius_inner = '60.0 80.0 100.0 120.0'
  radius_outer = '80.0 100.0 120.0 140.0'
  temperature = temp
  incremental = true
  eigenstrain_names = thermal_expansion
[]

[Modules/TensorMechanics/Master]
  [./master]
    strain = FINITE
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
    planar_formulation = PLANE_STRAIN
    eigenstrain_names = thermal_expansion
  [../]
[]

[AuxKernels]
  [./SED]
    type = MaterialRealAux
    variable = SED
    property = strain_energy_density
    execute_on = timestep_end
  [../]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = tempfunc
    block = 1
  [../]
[]

[BCs]
  [./crack_y]
    type = DirichletBC
    variable = disp_y
    boundary = 100
    value = 0.0
  [../]

  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 400
    value = 0.0
  [../]

  [./no_x1]
    type = DirichletBC
    variable = disp_x
    boundary = 900
    value = 0.0
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
  [./thermal_expansion_strain]
    type = ComputeInstantaneousThermalExpansionFunctionEigenstrain
    block = 1
    thermal_expansion_function = cte_func
    stress_free_temperature = 0.0
    temperature = temp
    eigenstrain_name = thermal_expansion
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm         31   preonly   lu      1'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 40

  nl_rel_step_tol= 1e-10
  nl_rel_tol = 1e-10

  start_time = 0.0
  dt = 1

  end_time = 1
  num_steps = 1
[]

[Outputs]
  csv = true
[]

[Preconditioning]
  [./smp]
    type = SMP
    pc_side = left
    ksp_norm = preconditioned
    full = true
  [../]
[]
