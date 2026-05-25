# Verify that residual_saturation is correctly passed to CapillaryPressureVG
# when using the PorousFlowUnsaturated action.
# Test that residual saturation is correctly applied to the saturation calculation
# With alpha=1E-6, m=0.6, s_res=0.1, and pp=-1E6:
#   S_eff = (1 + (alpha*|pp|)^(1/(1-m)))^(-m) = 2^(-0.6) ~= 0.6598
#   S     = s_res + S_eff*(1 - s_res)          = 0.1 + 0.6598*0.9 ~= 0.6938

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [pp]
    [InitialCondition]
      type = ConstantIC
      value = -1E6
    []
  []
[]

[BCs]
  [fixed_pp]
    type = DirichletBC
    variable = pp
    boundary = 'left right'
    value = -1E6
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[PorousFlowUnsaturated]
  porepressure = pp
  dictator_name = dictator
  fp = simple_fluid
  van_genuchten_alpha = 1E-6
  van_genuchten_m = 0.6
  residual_saturation = 0.1
  add_darcy_aux = false
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-13 0 0  0 1E-13 0  0 0 1E-13'
  []
[]

[Postprocessors]
  [saturation]
    type = PointValue
    variable = saturation0
    point = '0.5 0 0'
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
  csv = true
[]
