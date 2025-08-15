[Mesh]
  # Single QUAD4 element; initialize displacement in a pattern heavily aligned with g2
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
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

[Variables]
  [disp_x]
    [InitialCondition]
      type = FunctionIC
      # f(x,y) = 1 - 2*y produces nodal values [1,1,-1,-1]
      function = '1 - 2*y'
    []
  []
  [disp_y]
    [InitialCondition]
      type = FunctionIC
      function = 0
    []
  []
[]

[Kernels]
  [hourglass_x]
    type = HourglassCorrectionQuad4b
    variable = disp_x
    penalty = 1
    shear_modulus = 1
  []
  [hourglass_y]
    type = HourglassCorrectionQuad4b
    variable = disp_y
    penalty = 1
    shear_modulus = 1
  []
[]

[BCs]
  # Minimal anchoring to avoid rigid-body modes and singular matrices
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
    type = ComputeFiniteStrainElasticStress
  []
  [stiffness]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0.3
  []
[]

[Problem]
  kernel_coverage_check = FALSE
[]

[Executioner]
  type = Steady

  [Quadrature]
    type = GAUSS
    order = CONSTANT
  []
[]

[VectorPostprocessors]
  [nodes]
    type = NodalValueSampler
    sort_by = X
    variable = 'disp_x disp_y'
    execute_on = 'INITIAL FINAL'
  []
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL FINAL'
[]
