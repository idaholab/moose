# This example calculates the effective thermal conductivity across a microstructure
# with circular second phase precipitates. Two methods are used to calculate the effective thermal conductivity,
# the direct method that applies a temperature to one side and a heat flux to the other,
# and the AEH method.
[Mesh] #Sets mesh size to 10 microns by 10 microns
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    ny = 100
    xmax = 10
    ymax = 10
  []
  [new_nodeset]
    input = gen
    type = ExtraNodesetGenerator
    coord = '5 5'
    new_boundary = 100
  []
[]

[Variables] #Adds variables needed for two ways of calculating effective thermal cond.
  [T] #Temperature used for the direct calculation
    initial_condition = 800
  []
  [Tx_AEH] #Temperature used for the x-component of the AEH solve
    initial_condition = 800
    scaling = 1.0e4 #Scales residual to improve convergence
  []
  [Ty_AEH] #Temperature used for the y-component of the AEH solve
    initial_condition = 800
    scaling = 1.0e4  #Scales residual to improve convergence
  []
[]

[AuxVariables] #Creates second constant phase
  [phase2]
  []
[]

[ICs] #Sets the IC for the second constant phase
  [phase2_IC] #Creates circles with smooth interfaces at random locations
    variable = phase2
    type = MultiSmoothCircleIC
    int_width = 0.3
    numbub = 20
    bubspac = 1.5
    radius = 0.5
    outvalue = 0
    invalue = 1
    block = 0
  []
[]

[Kernels]
  [HtCond] #Kernel for direct calculation of thermal cond
    type = HeatConduction
    variable = T
  []
  [heat_x] #All other kernels are for AEH approach to calculate thermal cond.
    type = HeatConduction
    variable = Tx_AEH
  []
  [heat_rhs_x]
    type = HomogenizedHeatConduction
    variable = Tx_AEH
    component = 0
  []
  [heat_y]
    type = HeatConduction
    variable = Ty_AEH
  []
  [heat_rhs_y]
    type = HomogenizedHeatConduction
    variable = Ty_AEH
    component = 1
  []
[]

[BCs]
  [Periodic]
    [all]
      auto_direction = 'x y'
      variable = 'Tx_AEH Ty_AEH'
    []
  []
  [left_T] #Fix temperature on the left side
    type = DirichletBC
    variable = T
    boundary = left
    value = 800
  []
  [right_flux] #Set heat flux on the right side
    type = NeumannBC
    variable = T
    boundary = right
    value = 5e-6
  []
  [fix_x] #Fix Tx_AEH at a single point
    type = DirichletBC
    variable = Tx_AEH
    value = 800
    boundary = 100
  []
  [fix_y] #Fix Ty_AEH at a single point
    type = DirichletBC
    variable = Ty_AEH
    value = 800
    boundary = 100
  []
[]

[Materials]
  [thcond] #The equation defining the thermal conductivity is defined here, using two ifs
    # The k in the bulk is k_b, in the precipitate k_p2, and across the interaface k_int
    type = ParsedMaterial
    block = 0
    constant_names = 'length_scale k_b k_p2 k_int'
    constant_expressions = '1e-6 5 1 0.1'
    expression = 'sk_b:= length_scale*k_b; sk_p2:= length_scale*k_p2; sk_int:= k_int*length_scale; if(phase2>0.1,if(phase2>0.95,sk_p2,sk_int),sk_b)'
    outputs = exodus
    f_name = thermal_conductivity
    coupled_variables = phase2
  []
[]

[Postprocessors]
  [right_T]
    type = SideAverageValue
    variable = T
    boundary = right
  []
  [k_x_direct] #Effective thermal conductivity from direct method
    # This value is lower than the AEH value because it is impacted by second phase
    # on the right boundary
    type = ThermalConductivity
    variable = T
    flux = 5e-6
    length_scale = 1e-06
    T_hot = 800
    dx = 10
    boundary = right
  []
  [k_x_AEH] #Effective thermal conductivity in x-direction from AEH
    type = HomogenizedThermalConductivity
    chi = 'Tx_AEH Ty_AEH'
    row = 0
    col = 0
    scale_factor = 1e6 #Scale due to length scale of problem
  []
  [k_y_AEH] #Effective thermal conductivity in x-direction from AEH
    type = HomogenizedThermalConductivity
    chi = 'Tx_AEH Ty_AEH'
    row = 1
    col = 1
    scale_factor = 1e6 #Scale due to length scale of problem
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    off_diag_row = 'Tx_AEH Ty_AEH'
    off_diag_column = 'Ty_AEH Tx_AEH'
  []
[]

[Executioner]
  type = Steady
  l_max_its = 15
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -pc_hypre_boomeramg_strong_threshold'
  petsc_options_value = 'hypre boomeramg 31 0.7'
  l_tol = 1e-04
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  csv = true
[]
