# A fully-confined aquifer is fully saturated with water
# Barometric loading is applied to the aquifer.
# Because the aquifer is assumed to be sandwiched between
# impermeable aquitards, the barometric pressure is not felt
# directly by the porepressure.  Instead, the porepressure changes
# only because the barometric loading applies a total stress to
# the top surface of the aquifer.
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
  [fix_x]
    type = DirichletBC
    variable = disp_x
    value = 0.0
    boundary = 'left right'
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    value = 0.0
    boundary = 'bottom top'
  []
  [fix_z_bottom]
    type = DirichletBC
    variable = disp_z
    value = 0.0
    boundary = back
  []
  [barometric_loading]
    type = FunctionNeumannBC
    variable = disp_z
    function = -1000.0 # atmospheric pressure increase of 1kPa
    boundary = front
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
  dt = 1
  end_time = 1
[]

[Outputs]
  console = true
  csv = true
[]
