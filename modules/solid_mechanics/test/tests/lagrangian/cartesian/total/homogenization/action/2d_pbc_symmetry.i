# 2D thermomechanical coupling
# strain periodicity in +-x
# symmetry about the -y plane (i.e. bottom)

[GlobalParams]
  displacements = 'disp_x disp_y'
  large_kinematics = true
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
  []
  [pins]
    type = ExtraNodesetGenerator
    input = 'gmg'
    new_boundary = 'pin'
    coord = '0 0 0'
  []
[]

[Variables]
  [T]
    initial_condition = 300
  []
[]

[Kernels]
  [htime]
    type = CoefTimeDerivative
    variable = 'T'
    Coefficient = 1
  []
  [hcond]
    type = MatDiffusion
    variable = 'T'
    diffusivity = 1e-4
  []
  [hsource]
    type = MatBodyForce
    variable = 'T'
    material_property = source
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = FINITE
        add_variables = true
        new_system = true
        formulation = TOTAL
        volumetric_locking_correction = false
        temperature = T
        eigenstrain_names = 'thermal_egs'
        constraint_types = 'strain none none none none none none none none'
        targets = '0'
        generate_output = 'strain_xx strain_xy strain_yy'
      []
    []
  []
[]

[BCs]
  [Periodic]
    [x]
      variable = 'disp_x'
      auto_direction = 'x'
    []
    [y]
      variable = 'disp_y'
      auto_direction = 'x'
    []
    [T]
      variable = 'T'
      auto_direction = 'x'
    []
  []
  # remove rigid body modes
  [fix_x]
    type = DirichletBC
    boundary = 'pin'
    variable = 'disp_x'
    value = 0
  []
  # symmetry
  [symmetry]
    type = DirichletBC
    boundary = 'bottom'
    variable = 'disp_y'
    value = 0
  []
[]

[Functions]
  [source]
    type = ParsedFunction
    expression = '3*sin(2*pi*x)-y^2'
  []
[]

[Materials]
  [source]
    type = GenericFunctionMaterial
    prop_names = 'source'
    prop_values = 'source'
  []
  [thermal_egs]
    type = ComputeThermalExpansionEigenstrain
    eigenstrain_name = 'thermal_egs'
    stress_free_temperature = 300
    thermal_expansion_coeff = 5e-3
    temperature = 'T'
  []
  [C]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e5
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLagrangianLinearElasticStress
    objective_rate = jaumann
  []
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'
  line_search = none
  automatic_scaling = true

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_max_its = 10
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  dt = 1
  end_time = 2
[]

[Outputs]
  exodus = true
[]
