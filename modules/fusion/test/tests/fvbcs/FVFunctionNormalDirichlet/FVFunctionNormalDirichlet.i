# This test uses a cutsom boundary condition "FVFunctionNormalDirichlet" on a 1D line. This custom object is intended to be used for the porous flow model
# along the navier stokes module and provides a boundary condtion for the normal. The results of this case was visually checked in paraview and used as a gold
# file for testing.

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
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1
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
    mu = 1
    porosity = porosity
    momentum_component = 'x'
  []
[]

[FVBCs]
  [right]
    type = FVFunctionalNormalDirichletBC
    variable = pressure
    direction = x
    function = '1'
    boundary = 'right'
  []
[]

[Materials]
[]

