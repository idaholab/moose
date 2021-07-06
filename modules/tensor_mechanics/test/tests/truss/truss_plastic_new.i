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
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  # [axial_stress]
  #   order = CONSTANT
  #   family = MONOMIAL
  # []
  [e_over_l]
    order = CONSTANT
    family = MONOMIAL
  []
  # [forces]
  #   order = CONSTANT
  #   family = MONOMIAL
  # []
  [area]
    order = CONSTANT
    family = MONOMIAL
  []
  [react_x]
    order = FIRST
    family = LAGRANGE
  []
[]

[Functions]
  [hf]
    type = PiecewiseLinear
    x = '0    0.0001  0.0003  0.0023'
    y = '50e6 52e6    54e6    56e6'
  []
[]

[BCs]
  [fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [load]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = 't'
  []
[]

[AuxKernels]
  [e_over_l]
    type = MaterialRealAux
    property = e_over_l
    variable = e_over_l
  []
  [area]
    type = ConstantAux
    variable = area
    value = 1
    execute_on = 'initial timestep_begin'
  []
[]

[Postprocessors]
  [s_xx]
    type = ElementIntegralMaterialProperty
    mat_prop = axial_stress
  []
  [ep_xx]
    type = ElementIntegralMaterialProperty
    mat_prop = plastic_strain
  []
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
  [solid]
    type = StressDivergenceTruss
    component = 0
    variable = disp_x
    save_in = react_x
  []
[]

[Materials]
  [elasticity]
    type = ComputeElasticityTruss
    youngs_modulus = 2e11
  []
  [strain]
    type = ComputeIncrementalTrussStrain
    displacements = 'disp_x'
    area = area
  []
  [truss]
    type = ComputePlasticTrussResultants
    area = area
    yield_stress = 500e5
    hardening_constant = 0.
    outputs = exodus
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
