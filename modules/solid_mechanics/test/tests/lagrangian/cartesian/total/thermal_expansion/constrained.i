[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = false
  eigenstrain_names = "thermal_contribution"
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[AuxVariables]
  [temperature]
  []
[]

[AuxKernels]
  [control_temperature]
    type = FunctionAux
    variable = temperature
    function = temperature_control
  []
[]

[BCs]
  [leftx]
    type = DirichletBC
    preset = true
    boundary = left
    variable = disp_x
    value = 0.0
  []
  [rightx]
    type = DirichletBC
    preset = true
    boundary = right
    variable = disp_x
    value = 0.0
  []
  [lefty]
    type = DirichletBC
    preset = true
    boundary = bottom
    variable = disp_y
    value = 0.0
  []
  [leftz]
    type = DirichletBC
    preset = true
    boundary = back
    variable = disp_z
    value = 0.0
  []
[]

[Functions]
  [temperature_control]
    type = ParsedFunction
    expression = '100*t'
  []
[]

[Modules]
  [TensorMechanics]
    [Master]
      [all]
        strain = SMALL
        new_system = true
        formulation = TOTAL
        volumetric_locking_correction = false
        generate_output = 'cauchy_stress_xx cauchy_stress_yy cauchy_stress_zz cauchy_stress_xy '
                          'cauchy_stress_xz cauchy_stress_yz strain_xx strain_yy strain_zz strain_xy '
                          'strain_xz strain_yz'
      []
    []
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
  [thermal_expansion]
    type = ComputeThermalExpansionEigenstrain
    temperature = temperature
    thermal_expansion_coeff = 1.0e-3
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
  end_time = 1
  dt = 1
  type = Transient

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
[]
