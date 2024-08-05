# This input file tests inlet, wall, symmetry, and outflow boundary conditions
# for a conservative weak form of the incompressible NS equations
U_in = 1
mu = 1e-3
rho = 1

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

[ICs]
  [vel]
    type = VectorConstantIC
    variable = vel
    x_value = ${U_in}
  []
[]

[AuxVariables]
  [mixing_length]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [mixing_length]
    type = WallDistanceMixingLengthAux
    walls = 'top'
    variable = mixing_length
    execute_on = 'initial'
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
  [momentum_eddy]
    type = INSADSmagorinskyEddyViscosity
    variable = vel
    mixing_length = mixing_length
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
    prop_values = '${rho}  ${mu}'
  []
  [mat]
    type = INSADMaterial
    velocity = vel
    pressure = p
  []
[]

[Preconditioning]
  [FSP]
    type = FSP
    topsplit = 'by_diri_others'
    [by_diri_others]
      splitting = 'diri others'
      splitting_type  = additive
      petsc_options_iname = '-ksp_type'
      petsc_options_value = 'preonly'
    []
      [diri]
        sides = 'top'
        vars = 'vel'
        petsc_options_iname = '-ksp_type'
        petsc_options_value = 'none'
      []
      [others]
        splitting = 'u p'
        unside_by_var_boundary_name = 'top'
        unside_by_var_var_name = 'vel'
      []
        [u]
          vars = 'vel'
          unside_by_var_boundary_name = 'top'
          unside_by_var_var_name = 'vel'
        []
        [p]
          vars = 'p'
        []
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]

[Functions]
  [inlet]
    type = ParsedVectorFunction
    expression_x = ${U_in}
  []
  [walls]
    type = ParsedVectorFunction
  []
[]
