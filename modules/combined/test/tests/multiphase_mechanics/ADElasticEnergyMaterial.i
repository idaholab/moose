# Derived from (modules/combined/test/tests/multiphase_mechanics/elasticenergymaterial.i)

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  nz = 0
  xmax = 5
  ymax = 5
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [c]
    [InitialCondition]
      type = SmoothCircleIC
      x1 = 125.0
      y1 = 125.0
      radius = 60.0
      invalue = 1.0
      outvalue = 0.1
      int_width = 50.0
    []
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  []
  [left]
    type = DirichletBC
    boundary = left
    variable = disp_x
    value = 0.0
  []
[]

[Kernels]
  [dummy]
    type = MatDiffusion
    variable = c
    diffusivity = 0
  []
  [c_dt]
    type = TimeDerivative
    variable = c
  []
[]

# Needed as Tensor Mechanics does not have an AD version
[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        add_variables = true
        strain = SMALL
        eigenstrain_names = 'eigenstrain'
        automatic_eigenstrain_names = false
        use_automatic_differentiation = true
      []
    []
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '3 1 1 3 1 3 1 1 1 '
  []
  [stress]
    type = ADComputeLinearElasticStress
  []
  [prefactor]
    type = ADDerivativeParsedMaterial
    coupled_variables = c
    property_name = prefactor
    constant_names = 'epsilon0 c0'
    constant_expressions = '0.05     0'
    expression = '(c - c0) * epsilon0'
  []
  [eigenstrain]
    type = ADComputeVariableEigenstrain
    eigen_base = '1'
    args = c
    prefactor = prefactor
    eigenstrain_name = eigenstrain
  []
  [elasticenergy]
    type = ADElasticEnergyMaterial
    outputs = exodus
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  nl_abs_tol = 1e-10
  num_steps = 1

  petsc_options_iname = '-pc_factor_shift_type -pc_type'
  petsc_options_value = 'nonzero lu'
[]

[Outputs]
  exodus = true
[]
