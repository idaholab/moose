# Flow and solute transport along a fracture embedded in a porous matrix
# The fracture is represented by lower dimensional elements
# fracture aperture = 6e-4m
# fracture porosity = 6e-4m = phi * a
# fracture permeability = 1.8e-11 which is based on k=3e-8 from a**2/12, and k*a = 3e-8*6e-4
# matrix porosity = 0.1
# matrix permeanility = 1e-20

[Mesh]
  type = FileMesh
  file = 'coarse.e'
  block_id = '1 2 3'
  block_name = 'fracture matrix1 matrix2'

  boundary_id = '1 2'
  boundary_name = 'bottom top'
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [pp]
  []
  [massfrac0]
  []
[]

[AuxVariables]
  [velocity_x]
    family = MONOMIAL
    order = CONSTANT
    block = 'fracture'
  []
  [velocity_y]
    family = MONOMIAL
    order = CONSTANT
    block = 'fracture'
  []
[]

[AuxKernels]
  [velocity_x]
    type = PorousFlowDarcyVelocityComponentLowerDimensional
    variable = velocity_x
    component = x
    aperture = 6E-4
  []
  [velocity_y]
    type = PorousFlowDarcyVelocityComponentLowerDimensional
    variable = velocity_y
    component = y
    aperture = 6E-4
  []
[]

[ICs]
  [massfrac0]
    type = ConstantIC
    variable = massfrac0
    value = 0
  []
  [pp_matrix]
    type = ConstantIC
    variable = pp
    value = 1E6
  []
[]

[BCs]
  [top]
    type = DirichletBC
    value = 0
    variable = massfrac0
    boundary = top
  []
  [bottom]
    type = DirichletBC
    value = 1
    variable = massfrac0
    boundary = bottom
  []
  [ptop]
    type = DirichletBC
    variable = pp
    boundary =  top
    value = 1e6
  []
  [pbottom]
    type = DirichletBC
    variable = pp
    boundary = bottom
    value = 1.002e6
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = pp
  []
  [adv0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = pp
  []
  [diff0]
    type = PorousFlowDispersiveFlux
    fluid_component = 1
    variable = pp
    disp_trans = 0
    disp_long = 0
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = massfrac0
  []
  [adv1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = massfrac0
  []
  [diff1]
    type = PorousFlowDispersiveFlux
    fluid_component = 0
    variable = massfrac0
    disp_trans = 0
    disp_long = 0
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp massfrac0'
    number_fluid_phases = 1
    number_fluid_components = 2
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    thermal_expansion = 0
    viscosity = 1e-3
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = massfrac0
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [poro_fracture]
    type = PorousFlowPorosityConst
    porosity = 6e-4   # = a * phif
    block = 'fracture'
  []
  [poro_matrix]
    type = PorousFlowPorosityConst
    porosity = 0.1
    block = 'matrix1 matrix2'
  []
  [diff1]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '1e-9 1e-9'
    tortuosity = 1.0
    block = 'fracture'
  []
  [diff2]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '1e-9 1e-9'
    tortuosity = 0.1
    block = 'matrix1 matrix2'
  []
  [permeability_fracture]
    type = PorousFlowPermeabilityConst
    permeability = '1.8e-11 0 0 0 1.8e-11 0 0 0 1.8e-11'   # 1.8e-11 = a * kf
    block = 'fracture'
  []
  [permeability_matrix]
    type = PorousFlowPermeabilityConst
    permeability = '1e-20 0 0 0 1e-20 0 0 0 1e-20'
    block = 'matrix1 matrix2'
  []
  [relp]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
  []
[]

[Preconditioning]
  [basic]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  end_time = 10
  dt = 1

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-14
  nl_abs_tol = 1e-12

[]

[VectorPostprocessors]
  [xmass]
    type = LineValueSampler
    start_point = '-0.5 0 0'
    end_point = '0.5 0 0'
    sort_by = x
    num_points = 41
    variable = massfrac0
    outputs = csv
  []
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = 'final'
  []
[]
