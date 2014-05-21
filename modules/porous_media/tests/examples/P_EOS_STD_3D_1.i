[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 1
  nz = 1
  xmax = 10
  ymax = 1
  zmax = 1
[]

[Variables]
  [./pressure]
    initial_condition = 0.1e6
  [../]
[]

[AuxVariables]
 #----this aukernel is not required for computation----#
  [./v_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
 #----Pressure: diffusion, time derivative----#
  [./p_wmfp]
    type = WaterMassFluxPressure_PT
    variable = pressure
  [../]
[]

[AuxKernels]
 #----this aukernel is not required for computation----#
 [./vx]
    type = VelocityAux
    variable = v_x
    component = 0
  [../]
[]

[BCs]
#----the NeumannBC value of 1 indicates a mass flux of 1 kg/s entering at the left face----#
 [./left_p]
    type = NeumannBC
    variable = pressure
    boundary = left
    value = 1.0
  [../]
  [./right_p]
    type = DirichletBC
    variable = pressure
    boundary = right
    value = 0.1e6
  [../]
[]

[Materials]
  [./rock]
    type = Geothermal
    block = 0
    pressure = pressure

    temp_dependent_fluid_props = false

    gravity = 0.0
    gx = 0.0
    gy = 0.0
    gz = 1.0

    porosity = 0.1
    permeability = 1.0e-10
  [../]
[]

[Executioner]
  type = Steady
 #num_steps = 200
  #dt = 1
  nl_abs_tol = 1e-11
  [./Quadrature]
    type = Trap
  [../]
[]

[Outputs]
  file_base = P_EOS_STD_3d_1_out
  output_initial = true
  exodus = true
  console = true
[]
