a = 10
b = 10
c = 10
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [temperature]
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
    temperature = temperature
    eigenstrain_names = "thermal_contribution"
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
    temperature = temperature
    eigenstrain_names = "thermal_contribution"
  []
  [sdz]
    type = TotalLagrangianStressDivergence
    variable = disp_z
    component = 2
    temperature = temperature
    eigenstrain_names = "thermal_contribution"
  []
  [temperature]
    type = ADDiffusion
    variable = temperature
    use_displaced_mesh = true
  []
[]
[BCs]
  [leftx]
    type = DirichletBC
    boundary = top
    variable = disp_x
    value = 0.0
  []
  [rightx]
    type = DirichletBC
    boundary = top
    variable = disp_y
    value = 0.0
  []
  [lefty]
    type = DirichletBC
    boundary = top
    variable = disp_z
    value = 0.0
  []

  [bottom_t]
    type = FunctionDirichletBC
    preset = false
    boundary = bottom
    variable = temperature
    function = '(${a}*x+${b}*y+${c}*x*y)'
  []
  [top_t]
    type = DirichletBC
    boundary = top
    variable = temperature
    value = 0
  []
[]
[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 100000.0
    poissons_ratio = 0.3
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [compute_strain]
    type = ComputeLagrangianStrain
    eigenstrain_names = "thermal_contribution"
  []
  [thermal_expansion]
    type = ComputeThermalExpansionEigenstrain
    temperature = temperature
    thermal_expansion_coeff = 1.0e-2
    eigenstrain_name = thermal_contribution
    stress_free_temperature = 0.0
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type '
  petsc_options_value = 'lu       NONZERO               '
  end_time = 1
  dt = 1
  residual_and_jacobian_together = true
  type = Transient
[]

[Outputs]
  exodus = true
[]

[Problem]
  solve = false
[]

[VariableMappings]
  [rb_mapping]
    type = DEIMRBMapping
    filename = 'parallel_storage_main_mapping_rb_mapping.rd'
  []
[]

[UserObjects]
  [im]
    type = InverseRB
    mapping = rb_mapping
    execute_on = 'INITIAL TIMESTEP_END'
    relaxation_factor = 0.25
    max_iter = 20
  []
[]

[Outputs]
  console = true
  # exodus = true
  [perf]
    type = PerfGraphOutput
    level = 3
  []
[]
