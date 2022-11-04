# Testing the UMAT Interface - creep linear strain hardening model using the finite strain formulation - visco-plastic material.
# Uses 2D plane strain

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
  []
[]

[Functions]
  [top_pull]
    type = ParsedFunction
    expression = t/100
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    generate_output = 'strain_yy stress_yy stress_zz'
    planar_formulation = PLANE_STRAIN
  []
[]

[BCs]
  [y_pull_function]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = top_pull
  []
  [x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
[]

[Materials]
  [constant]
    type = AbaqusUMATStress
    #                      Young's modulus,  Poisson's Ratio, Yield, Hardening
    constant_properties = '1000 0.3 10 100'
    plugin = ../../../plugins/linear_strain_hardening
    num_state_vars = 3
    use_one_based_indexing = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9
  start_time = 0.0
  num_steps = 30
  dt = 1.0
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [average_strain_yy]
    type = ElementAverageValue
    variable = 'strain_yy'
  []
  [average_stress_yy]
    type = ElementAverageValue
    variable = 'stress_yy'
  []
  [average_stress_zz]
    type = ElementAverageValue
    variable = 'stress_zz'
  []
[]

[Outputs]
  [out]
    type = Exodus
    elemental_as_nodal = true
  []
[]
