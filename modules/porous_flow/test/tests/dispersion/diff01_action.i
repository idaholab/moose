# Test diffusive part of PorousFlowDispersiveFlux kernel by setting dispersion
# coefficients to zero. Pressure is held constant over the mesh, and gravity is
# set to zero so that no advective transport of mass takes place.
# Mass fraction is set to 1 on the left hand side and 0 on the right hand side.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmax = 10
  bias_x = 1.1
[]

[GlobalParams]
  PorousFlowDictator = andy_heheheh
[]

[Variables]
  [pp]
  []
  [massfrac0]
  []
[]

[ICs]
  [pp]
    type = ConstantIC
    variable = pp
    value = 1e5
  []
  [massfrac0]
    type = ConstantIC
    variable = massfrac0
    value = 0
  []
[]

[BCs]
  [left]
    type = DirichletBC
    value = 1
    variable = massfrac0
    boundary = left
  []
  [right]
    type = DirichletBC
    value = 0
    variable = massfrac0
    boundary = right
  []
  [pright]
    type = DirichletBC
    variable = pp
    boundary = right
    value = 1e5
  []
  [pleft]
    type = DirichletBC
    variable = pp
    boundary = left
    value = 1e5
  []
[]

[Kernels]
  [diff0]
    type = PorousFlowDispersiveFlux
    fluid_component = 0
    variable = massfrac0
    disp_trans = 0
    disp_long = 0
    gravity = '0 0 0'
  []
  [diff1]
    type = PorousFlowDispersiveFlux
    fluid_component = 1
    variable = pp
    disp_trans = 0
    disp_long = 0
    gravity = '0 0 0'
  []
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 0.0
    bulk_modulus = 1E7
    viscosity = 0.001
    density0 = 1000.0
  []
[]

[PorousFlowUnsaturated]
  porepressure = pp
  gravity = '0 0 0'
  fp = the_simple_fluid
  dictator_name = andy_heheheh
  relative_permeability_type = Corey
  relative_permeability_exponent = 0.0
  mass_fraction_vars = massfrac0
[]

[Materials]
  [poro]
    type = PorousFlowPorosityConst
    porosity = 0.3
  []
  [diff]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '1 1'
    tortuosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-9 0 0 0 1e-9 0 0 0 1e-9'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2             '
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1
  end_time = 20
[]

[VectorPostprocessors]
  [xmass]
    type = NodalValueSampler
    sort_by = id
    variable = massfrac0
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = final
  []
[]
