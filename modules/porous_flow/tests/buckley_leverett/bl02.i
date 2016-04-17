# Buckley-Leverett 1-phase with the MD formulation
# (where the primary variable is log(mass-density)
# The front starts at (around) x=5, and at t=50 it should
# have moved to x=9.6.
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 150
  xmin = 0
  xmax = 15
[]

[GlobalParams]
  PorousFlowDictator_UO = dictator
[]

[Variables]
  [./md]
    [./InitialCondition]
      type = FunctionIC
      # want pp = 'max((1000000-x/5*1000000)-20000,-20000)' .
      # In the saturated zone
      # md = log(density0) + porepressure/bulk
      function = '6.907755278982137+max((1000000-x/5*1000000)-20000,-20000)/2E6-if(max((1000000-x/5*1000000)-20000,-20000)>0,0,max((1000000-x/5*1000000)-20000,-20000)*max((1000000-x/5*1000000)-20000,-20000)*1E-4*1E-4)'
      # following is sharp in md
      #function = 'if(x<5,6.907755278982137+(1000000-20000)/2E6,-0.1)'
      # following is fully saturated
      #function = '6.907755278982137+max((1000000-x/5*1000000)-0,-0)/2E6-if(max((1000000-x/5*1000000)-0,-0)>0,0,max((1000000-x/5*1000000)-0,-0)*max((1000000-x/5*1000000)-0,-0)*1E-4*1E-4)'
    [../]
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    component_index = 0
    variable = md
  [../]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    component_index = 0
    variable = md
    gravity = '0 0 0'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = md
    boundary = left
    # want pp = 980000
    value = 7.39775527898214
  [../]
[]

[AuxVariables]
  [./sat]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./pp_material]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./bounds_dummy]
  [../]
[]

[AuxKernels]
  [./sat]
    type = MaterialStdVectorAux
    variable = sat
    execute_on = timestep_end
    index = 0
    property = PorousFlow_saturation_qp
  [../]
  [./pp_material]
    type = MaterialStdVectorAux
    variable = pp_material
    execute_on = timestep_end
    index = 0
    property = PorousFlow_porepressure_qp
  [../]
[]

[Bounds]
  [./pwater_bounds]
    type = BoundsAux
    variable = bounds_dummy
    bounded_variable = md
    upper = 7.0 #7.6
    lower = 2.5
  [../]
    

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'md'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Materials]
  [./ppss]
    type = PorousFlowMaterial1PhaseMD_Gaussian
    mass_density = md
    al = 1E-4 #(0.018)
    density0 = 1000
    bulk_modulus = 2E6
  [../]
  [./massfrac]
    type = PorousFlowMaterialMassFractionBuilder
  [../]
  [./dens0]
    type = PorousFlowMaterialDensityConstBulk
    density0 = 1000
    bulk_modulus = 2E6
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowMaterialJoinerOld
    material_property = PorousFlow_fluid_phase_density
  [../]
  [./dens_qp_all]
    type = PorousFlowMaterialJoiner
    material_property = PorousFlow_fluid_phase_density_qp
  [../]
  [./visc0]
    type = PorousFlowMaterialViscosityConst
    viscosity = 1E-3
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowMaterialJoiner
    material_property = PorousFlow_viscosity
  [../]
  [./permeability]
    type = PorousFlowMaterialPermeabilityConst
    permeability = '1E-10 0 0  0 1E-10 0  0 0 1E-10'
  [../]
  [./relperm]
    type = PorousFlowMaterialRelativePermeabilityCorey
    n_j = 0
    phase = 0
  [../]
  [./relperm_all]
    type = PorousFlowMaterialJoiner
    material_property = PorousFlow_relative_permeability
  [../]
  [./porosity]
    type = PorousFlowMaterialPorosityConst
    porosity = 0.15
  [../]
[]


[Preconditioning]
  active = bounded
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres bjacobi 1E-10 1E-10 20'
  [../]
  [./bounded]
  # must use --use-petsc-dm command line argument
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type -ksp_rtol -ksp_atol'
    petsc_options_value = 'bcgs bjacobi 1E-7 1E-10 50 vinewtonssls 1E-5 1E-20'
  [../]
  [./experiment]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E-10 1E-10 200'
  [../]
[]

[Executioner]
  solve_type = NEWTON
  type = Transient
  line_search = bt
  end_time = 50
  dt = 1
[]

[Outputs]
  file_base = bl02
  execute_on = 'initial timestep_end final'
  exodus = true
[]
