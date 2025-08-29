[Mesh]
  # Single QUAD4 element; initialize displacement in the g1 hourglass pattern
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

[ICs]
  [ic_disp_x]
    type = FunctionIC
    variable = disp_x
    # g1 at nodes [1,-1,1,-1] is represented by f(x,y) = 1 - 2x - 2y + 4xy
    function = '1 - 2*x - 2*y + 4*x*y'
  []
  [ic_disp_y]
    type = FunctionIC
    variable = disp_y
    function = 0
  []
[]

[Kernels]
  # Use the advanced hourglass kernel; set mu=1 for simplicity here
  [hourglass_x]
    type = HourglassCorrectionQuad4
    variable = disp_x
    penalty = 1
    shear_modulus = 1
  []
  [hourglass_y]
    type = HourglassCorrectionQuad4
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
