[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 2
    ymax = 1
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    # we added this in the first exercise problem
    strain = FINITE
    # enable the use of automatic differentiation objects
    use_automatic_differentiation = true
  []
[]

[BCs]
  [bottom_x]
    # we use the AD version of this boundary condition here...
    type = ADDirichletBC
    variable = disp_x
    boundary = bottom
    value = 0
  []
  [bottom_y]
    # ...and here
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [Pressure]
    [top]
      boundary = top
      function = 1e7*t
      # make the action add AD versions of the boundary condition objects
      use_automatic_differentiation = true
    []
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e9
    poissons_ratio = 0.3
  []
  [stress]
    type = ADComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient
  # MOOSE automatically sets up SMP/full=true with NEWTON
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  end_time = 5
  dt = 1
[]

[Outputs]
  exodus = true
[]
