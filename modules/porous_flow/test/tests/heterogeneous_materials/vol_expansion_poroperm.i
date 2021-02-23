# Apply an increasing porepressure, with zero mechanical forces,
# and observe the corresponding volumetric expansion and porosity increase.
# Check that permeability is calculated correctly from porosity.
#
# P = t
# With the Biot coefficient being 1, the effective stresses should be
# stress_xx = stress_yy = stress_zz = t
# With bulk modulus = 1 then should have
# vol_strain = strain_xx + strain_yy + strain_zz = t.
#
# With the biot coefficient being 1, the porosity (phi) # at time t is:
# phi = 1 - (1 - phi0) / exp(vol_strain)
# where phi0 is the porosity at t = 0 and P = 0.
#
# The permeability (k) is
# k = k_anisotropic * f * d^2 * phi^n / (1-phi)^m

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
  block = 0
  PorousFlowDictator = dictator
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [p]
  []
[]

[BCs]
  [p]
    type = FunctionDirichletBC
    boundary = 'bottom top'
    variable = p
    function = t
  []
  [xmin]
    type = DirichletBC
    boundary = left
    variable = disp_x
    value = 0
  []
  [ymin]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0
  []
  [zmin]
    type = DirichletBC
    boundary = back
    variable = disp_z
    value = 0
  []
[]

[Kernels]
  [p_does_not_really_diffuse]
    type = Diffusion
    variable = p
  []
  [TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  []
  [poro_x]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 1
    variable = disp_x
    component = 0
  []
  [poro_y]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 1
    variable = disp_y
    component = 1
  []
  [poro_z]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 1
    variable = disp_z
    component = 2
  []
[]

[AuxVariables]
  [poro0]
    order = CONSTANT
    family = MONOMIAL
  []
  [poro]
    order = CONSTANT
    family = MONOMIAL
  []
  [perm_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [perm_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [perm_z]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[ICs]
  [poro0]
    type = RandomIC
    seed = 0
    variable = poro0
    max = 0.15
    min = 0.05
  []
[]

[AuxKernels]
  [poromat]
    type = PorousFlowPropertyAux
    property = porosity
    variable = poro
  []
  [perm_x]
    type = PorousFlowPropertyAux
    property = permeability
    variable = perm_x
    row = 0
    column = 0
  []
  [perm_y]
    type = PorousFlowPropertyAux
    property = permeability
    variable = perm_y
    row = 1
    column = 1
  []
  [perm_z]
    type = PorousFlowPropertyAux
    property = permeability
    variable = perm_z
    row = 2
    column = 2
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'p'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    bulk_modulus = 1
    shear_modulus = 1
  []
  [strain]
    type = ComputeSmallStrain
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [vol_strain]
    type = PorousFlowVolumetricStrain
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = p
    capillary_pressure = pc
  []
  [p_eff]
    type = PorousFlowEffectiveFluidPressure
  []
  [porosity]
    type = PorousFlowPorosity
    fluid = true
    mechanical = true
    porosity_zero = poro0
    solid_bulk = 1
    biot_coefficient = 1
  []
  [permeability]
    type = PorousFlowPermeabilityKozenyCarman
    k_anisotropy = '1 0 0  0 2 0  0 0 0.1'
    poroperm_function = kozeny_carman_fd2
    f = 0.1
    d = 5
    m = 2
    n = 7
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_atol -ksp_rtol'
    petsc_options_value = 'gmres bjacobi 1E-10 1E-10 10 1E-15 1E-10'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  start_time = 0
  dt = 0.1
  end_time = 1
[]

[Outputs]
  exodus = true
  execute_on = 'timestep_end'
[]
