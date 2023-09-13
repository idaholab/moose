# This tests uses the custom boundary condition "INFVInletVelcityNormalBC" that provides and inlet boundry condition to the normal for velocity. This case uses the object
# for the velocity variable alongside a 1D line. The results from this case was visually confirmed in paraview and used as a gold file for testing.

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolator
    u = superficial_vel_x
    pressure = pressure
    porosity = porosity
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[Outputs]
  exodus = true
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Variables]
  [pressure]
    type = INSFVPressureVariable
  []
  [superficial_vel_x]
    type = PINSFVSuperficialVelocityVariable
    #initial_condition = 2
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1000
  []
[]

[FVKernels]
  [pressure]
    type = PINSFVMassAdvection
    variable = pressure
    advected_interp_method = upwind
    velocity_interp_method = rc
    rho = 1
  []
  [u_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_vel_x
    mu = 0.0001
    porosity = porosity
    momentum_component = 'x'
  []
[]

[FVBCs]
  [right]
    type = INSFVInletVelocityNormalBC
    variable = superficial_vel_x
    direction = x
    function = 0.1
    boundary = 'right'
  []
  [left]
    type = INSFVInletVelocityNormalBC
    variable = superficial_vel_x
    direction = x
    function = 0.2
    boundary = 'left'
  []
  [right_pressure]
    type = FVFunctionalNormalDirichletBC
    variable = pressure
    direction = x
    function = 10000
    boundary = 'right'
  []
  [left_pressure]
    type = FVFunctionalNormalDirichletBC
    variable = pressure
    direction = x
    function = 20000
    boundary = 'left'
  []
[]

[Materials]
[]

