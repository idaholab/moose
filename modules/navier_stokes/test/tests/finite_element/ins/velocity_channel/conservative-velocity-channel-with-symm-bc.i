# This input file tests inlet, wall, symmetry, and outflow boundary conditions
# for a conservative weak form of the incompressible NS equations

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 3.0
    ymin = 0
    ymax = 1.0
    nx = 30
    ny = 10
    elem_type = QUAD9
  []
[]


[Variables]
  [vel]
    order = SECOND
    family = LAGRANGE_VEC
  []
  [p]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [mass]
    type = INSADConservativeMass
    variable = p
    velocity = vel
  []
  [momentum_convection]
    type = INSADMomentumConservativeAdvection
    variable = vel
    rho = rho
  []
  [momentum_viscous]
    type = INSADMomentumViscous
    variable = vel
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = vel
    pressure = p
    integrate_p_by_parts = true
  []
[]

[BCs]
  [walls_momentum]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = 'top'
    preset = true
  []
  [symm_momentum]
    type = INSADMomentumZeroViscousStreeBC
    variable = vel
    boundary = 'bottom'
    pressure = p
    integrate_p_by_parts = true
  []
  [inlet_mass]
    type = INSADConservativeMassWeakDiriBC
    variable = p
    velocity = 'inlet'
    boundary = 'left'
  []
  [inlet_momentum_advection]
    type = INSADMomentumConservativeAdvectionWeakDiriBC
    variable = vel
    rho = 'rho'
    velocity = 'inlet'
    boundary = 'left'
  []
  [inlet_momentum_stress]
    type = INSADMomentumZeroViscousStreeBC
    variable = vel
    boundary = 'left'
    pressure = p
    integrate_p_by_parts = true
  []
  [outlet_mass]
    type = INSADConservativeMassImplicitBC
    variable = p
    velocity = vel
    boundary = 'right'
  []
  [outlet_momentum]
    type = INSADMomentumConservativeAdvectionImplicitBC
    variable = vel
    rho = 'rho'
    boundary = 'right'
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '1  1'
  []
  [mat]
    type = INSADMaterial
    velocity = vel
    pressure = p
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       mumps'
  line_search = none
[]

[Outputs]
  exodus = true
[]

[Functions]
  [inlet]
    type = ParsedVectorFunction
    expression_x = 1
  []
  [walls]
    type = ParsedVectorFunction
  []
[]
