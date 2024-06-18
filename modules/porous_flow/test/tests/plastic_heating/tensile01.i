# Tensile heating, using capped weak-plane plasticity
# z_disp(z=1) = t
# totalstrain_zz = t
# with C_ijkl = 0.5 0.25
# stress_zz = t, but with tensile_strength = 1, stress_zz = min(t, 1)
# so plasticstrain_zz = t - 1
# heat_energy_rate = coeff * (t - 1)
# Heat capacity of rock = specific_heat_cap * density = 4
# So temperature of rock should be:
# (1 - porosity) * 4 * T = (1 - porosity) * coeff * (t - 1)
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
  [disp_z]
    type = FunctionAux
    variable = disp_z
    function = z*t
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
    value = 100
  []
  [tanphi]
    type = TensorMechanicsHardeningConstant
    value = 1.0
  []
  [t_strength]
    type = TensorMechanicsHardeningConstant
    value = 1
  []
  [c_strength]
    type = TensorMechanicsHardeningConstant
    value = 1
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
    porosity = 0.2
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
    tan_dilation_angle = tanphi
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
  file_base = tensile01
  csv = true
[]
