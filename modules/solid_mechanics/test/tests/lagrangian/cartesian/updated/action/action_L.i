[Mesh]
  type = FileMesh
  file = 'L.exo'
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules]
  [TensorMechanics]
    [Master]
      [all]
        strain = SMALL
        add_variables = true
        new_system = true
        formulation = UPDATED
        volumetric_locking_correction = true
        generate_output = 'cauchy_stress_xx cauchy_stress_yy cauchy_stress_zz cauchy_stress_xy '
                          'cauchy_stress_xz cauchy_stress_yz strain_xx strain_yy strain_zz strain_xy '
                          'strain_xz strain_yz'
      []
    []
  []
[]

[Functions]
  [pfn]
    type = PiecewiseLinear
    x = '0    1    2'
    y = '0.00 0.3 0.5'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    preset = true
    variable = disp_x
    boundary = fix
    value = 0.0
  []

  [bottom]
    type = DirichletBC
    preset = true
    variable = disp_y
    boundary = fix
    value = 0.0
  []

  [back]
    type = DirichletBC
    preset = true
    variable = disp_z
    boundary = fix
    value = 0.0
  []

  [front]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = pull
    function = pfn
    preset = true
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 100000.0
    poissons_ratio = 0.25
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  solve_type = 'newton'

  petsc_options_iname = -pc_type
  petsc_options_value = lu

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-8

  end_time = 1.0
  dtmin = 0.5
  dt = 0.5
[]

[Outputs]
  [out]
    type = Exodus
    file_base = 'blah'
  []
[]
