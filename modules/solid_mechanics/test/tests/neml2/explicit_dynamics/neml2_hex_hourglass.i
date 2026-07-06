# Reduced-integration 3D HEX8 explicit dynamics through the NEML2 nodal-force
# path with batched hourglass stabilization. Twin: moose_hex_hourglass.i.
!include 'expdyn3d.i'

[NEML2]
  input = '../elasticity/elasticity_neml2.i'
  [all]
    executor_name = 'neml2'
    model = 'model'
    verbose = false
    input_kernels = 'neml2_strain'
    auto_output = false
  []
[]

[UserObjects]
  [assembly]
    type = NEML2Assembly
  []
  [fe]
    type = NEML2FEInterpolation
    assembly = 'assembly'
  []
  [neml2_strain]
    type = NEML2SmallStrain
    assembly = 'assembly'
    fe = 'fe'
    to_neml2 = 'neml2_strain'
  []
  [residual]
    type = NEML2StressDivergence
    assembly = 'assembly'
    fe = 'fe'
    executor = 'neml2'
    stress = 'neml2_stress'
    residual = 'NONTIME'
  []
  [hourglass]
    type = NEML2HourglassCorrection
    assembly = 'assembly'
    fe = 'fe'
    executor = 'neml2'
    displacements = 'disp_x disp_y disp_z'
    penalty = 10
    shear_modulus = 1
    residual = 'NONTIME'
  []
[]

[Executioner]
  type = Transient
  [TimeIntegrator]
    type = NEML2CentralDifference
    mass_matrix_tag = 'mass'
    use_constant_mass = true
    second_order_vars = 'disp_x disp_y disp_z'
    assembly = 'assembly'
    fe = 'fe'
  []
  [Quadrature]
    type = GAUSS
    order = CONSTANT
  []
  start_time = 0.0
  num_steps = 30
  dt = 0.02
[]
