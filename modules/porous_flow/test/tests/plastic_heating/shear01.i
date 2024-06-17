# Tensile heating, using capped weak-plane plasticity
# x_disp(z=1) = t
# totalstrain_xz = t
# with C_ijkl = 0.5 0.25
# stress_zx = stress_xz = 0.25*t, so q=0.25*t, but
# with cohesion=1 and tan(phi)=1: max(q)=1.  With tan(psi)=0,
# the plastic return is always to (p, q) = (0, 1),
# so plasticstrain_zx = max(t - 4, 0)
# heat_energy_rate = coeff * (t - 4) for t>4
# Heat capacity of rock = specific_heat_cap * density = 4
# So temperature of rock should be:
# (1 - porosity) * 4 * T = (1 - porosity) * coeff * (t - 4)
[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -10
  xmax = 10
  zmin = 0
  zmax = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  PorousFlowDictator = dictator
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [energy_dot]
    type = PorousFlowEnergyTimeDerivative
    variable = temperature
    base_name = non_existent
  []
  [phe]
    type = PorousFlowPlasticHeatEnergy
    variable = temperature
    coeff = 8
  []
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[AuxKernels]
  [disp_x]
    type = FunctionAux
    variable = disp_x
    function = 'z*t'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = temperature
    number_fluid_phases = 0
    number_fluid_components = 0
  []
  [coh]
    type = TensorMechanicsHardeningConstant
    value = 1
  []
  [tanphi]
    type = TensorMechanicsHardeningConstant
    value = 1.0
  []
  [tanpsi]
    type = TensorMechanicsHardeningConstant
    value = 0.0
  []
  [t_strength]
    type = TensorMechanicsHardeningConstant
    value = 10
  []
  [c_strength]
    type = TensorMechanicsHardeningConstant
    value = 10
  []
[]

[Materials]
  [rock_internal_energy]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 2
    density = 2
  []
  [temp]
    type = PorousFlowTemperature
    temperature = temperature
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.7
  []
  [phe]
    type = ComputePlasticHeatEnergy
  []
  [elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0.5 0.25'
  []
  [strain]
    type = ComputeIncrementalStrain
    displacements = 'disp_x disp_y disp_z'
  []
  [admissible]
    type = ComputeMultipleInelasticStress
    inelastic_models = mc
    perform_finite_strain_rotations = false
  []
  [mc]
    type = CappedWeakPlaneStressUpdate
    cohesion = coh
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    tensile_strength = t_strength
    compressive_strength = c_strength
    tip_smoother = 0
    smoothing_tol = 1
    yield_function_tol = 1E-10
    perfect_guess = true
  []
[]

[Postprocessors]
  [temp]
    type = PointValue
    point = '0 0 0'
    variable = temperature
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 10
[]

[Outputs]
  file_base = shear01
  csv = true
[]
