[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 5
    xmax = 40
    ymax = 5
    elem_type = QUAD4
  []
  [point]
    type = ExtraNodesetGenerator
    input = gen
    new_boundary = fixpoint
    coord = '0 0 0'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        add_variables = true
        strain = FINITE
      []
    []
  []
[]

[Kernels]
  [gravity]
    type = Gravity
    value = 1e-5
    density = -1
    variable = disp_y
    function = t
  []
[]

[BCs]
  [left_x]
    type = DirichletBC
    boundary = left
    value = 0
    variable = disp_x
  []
  [fix_y]
    type = DirichletBC
    boundary = fixpoint
    value = 0
    variable = disp_y
  []
[]

[Materials]
  [stress]
    # type = ComputeLinearElasticStress
    type = ComputeFiniteStrainElasticStress
  []
  [stiffness]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0.3
  []
[]

[Kernels]
  [hourglass_x]
    type = HourglassCorrectionQuad4
    variable = disp_x
    penalty = 2e-3
  []
  [hourglass_y]
    type = HourglassCorrectionQuad4
    variable = disp_y
    penalty = 2e-3
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  num_steps = 40
  dt = 0.1

  [Quadrature]
    type = GAUSS
    order = CONSTANT
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
