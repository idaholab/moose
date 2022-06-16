[Mesh]
  type = GeneratedMesh
  dim = 1
  elem_type = EDGE
  nx = 1
[]

[GlobalParams]
  displacements = 'disp_x'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./axial_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e_over_l]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./area]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./react_x]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./hf]
    type = PiecewiseLinear
    x = '0    0.0001  0.0003  0.0023'
    y = '50e6 52e6    54e6    56e6'
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./load]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = 't'
  [../]
[]

[AuxKernels]
  [./axial_stress]
    type = MaterialRealAux
    property = axial_stress
    variable = axial_stress
    execute_on = 'initial TIMESTEP_END'
  [../]
  [./e_over_l]
    type = MaterialRealAux
    property = e_over_l
    variable = e_over_l
    execute_on = 'initial TIMESTEP_END'
  [../]
  [./area]
    type = ConstantAux
    variable = area
    value = 1.0
    execute_on = 'initial timestep_begin'
  [../]
[]

[Postprocessors]
  [./s_xx]
    type = ElementIntegralMaterialProperty
    mat_prop = axial_stress
  [../]
  [./e_xx]
    type = ElementIntegralMaterialProperty
    mat_prop = total_stretch
  [../]
  [./ee_xx]
    type = ElementIntegralMaterialProperty
    mat_prop = elastic_stretch
  [../]
  [./ep_xx]
    type = ElementIntegralMaterialProperty
    mat_prop = plastic_stretch
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-11
  l_max_its = 20
  dt = 5e-5
  num_steps = 10
[]

[Kernels]
  [./solid]
    type = StressDivergenceTensorsTruss
    component = 0
    variable = disp_x
    area = area
    save_in = react_x
  [../]
[]

[Materials]
  [./truss]
    type = PlasticTruss
    youngs_modulus = 2.0e11
    yield_stress = 500e5
    outputs = 'exodus'
    output_properties = 'elastic_stretch hardening_variable plastic_stretch total_stretch'
  [../]
[]

[Outputs]
  exodus = true
[]
