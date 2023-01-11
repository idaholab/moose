# A confined aquifer is fully saturated with water
# Earth tides apply strain to the aquifer and the resulting porepressure changes are recorded
#
# To replicate standard poroelasticity exactly:
# (1) the PorousFlowBasicTHM Action is used;
# (2) multiply_by_density = false;
# (3) PorousFlowConstantBiotModulus is used
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  PorousFlowDictator = dictator
  block = 0
  biot_coefficient = 0.6
  multiply_by_density = false
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [porepressure]
  []
[]

[BCs]
  [strain_x]
    type = FunctionDirichletBC
    variable = disp_x
    function = earth_tide_x
    boundary = 'left right'
  []
  [strain_y]
    type = FunctionDirichletBC
    variable = disp_y
    function = earth_tide_y
    boundary = 'bottom top'
  []
  [strain_z]
    type = FunctionDirichletBC
    variable = disp_z
    function = earth_tide_z
    boundary = 'back front'
  []
[]

[Functions]
  [earth_tide_x]
    type = ParsedFunction
    expression = 'x*1E-8*(5*cos(t*2*pi) + 2*cos((t-0.5)*2*pi) + 1*cos((t+0.3)*0.5*pi))'
  []
  [earth_tide_y]
    type = ParsedFunction
    expression = 'y*1E-8*(7*cos(t*2*pi) + 4*cos((t-0.3)*2*pi) + 7*cos((t+0.6)*0.5*pi))'
  []
  [earth_tide_z]
    type = ParsedFunction
    expression = 'z*1E-8*(7*cos((t-0.5)*2*pi) + 4*cos((t-0.8)*2*pi) + 7*cos((t+0.1)*4*pi))'
  []
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2E9
  []
[]

[PorousFlowBasicTHM]
  coupling_type = HydroMechanical
  displacements = 'disp_x disp_y disp_z'
  porepressure = porepressure
  gravity = '0 0 0'
  fp = the_simple_fluid
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    bulk_modulus = 10.0E9 # drained bulk modulus
    poissons_ratio = 0.25
  []
  [strain]
    type = ComputeSmallStrain
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [porosity]
    type = PorousFlowPorosityConst # only the initial value of this is ever used
    porosity = 0.1
  []
  [biot_modulus]
    type = PorousFlowConstantBiotModulus
    solid_bulk_compliance = 1E-10
    fluid_bulk_modulus = 2E9
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-12 0 0   0 1E-12 0   0 0 1E-12'
  []
[]

[Postprocessors]
  [pp]
    type = PointValue
    point = '0.5 0.5 0.5'
    variable = porepressure
  []
[]


[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.01
  end_time = 2
[]

[Outputs]
  console = true
  csv = true
[]
