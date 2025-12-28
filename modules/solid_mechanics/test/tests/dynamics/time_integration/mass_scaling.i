# Illustrating inertial mass scaling to enable a large critical time step in explicit time stepping

young = 3.6
poisson = 0.2
density_true = 16 # the true inertial density of the material

# The Courant condition is: critical timestep = l_min * sqrt(density) / sqrt(effective_stiffness)
# sqrt(effective_stiffness) = sqrt(3.6 * (1 - 0.2) / (1 + 0.2) / (1 - 2 * 0.2)) = 2
# sqrt(density) = 4
# Therefore, for the mesh below
# element1: critical timestep = 2
# element1: critical timestep = 4
# element1: critical timestep = 6
# element1: critical timestep = 8
# element1: critical timestep = 10
#
# Hence, with dt = 4, the explicit dynamics is unstable
#
# If we want to run with dt = 4, then mass must be scaled in the left-most elements.
# A new density, called density_scaled, must be set in the left-most elements.
# (It is actually set for all elements, and in the right-most elements it equals density_true.)
# Its numerical value is chosen so that the critical time step in those elements
# is 5, so using dt = 4 is safe.  Setting dt a little less than the critical
# time step is best-practice in explicit solid-mechanics simulations, eg, LS-DYNA
# scales mass so critical time-step is 1/0.7 times the time-step used.
#
# For the critical timestep to be 5 or greater in each element,
# sqrt(density_scaled) = 5 * sqrt(effective_stiffness) / l_min
# element1: density_scaled = (10 / 1)^2 = 100
# element2: density_scaled = (10 / 2)^2 = 50
# element3: density_scaled = (10 / 3)^2 = 11, but this is less than density_true, so set density_scaled = 16
# element4: density_scaled = 16
# element5: density_scaled = 16
#
# The input file achieves this in 2 small steps:
# (1) Use a DensityScaling Material with desired_time_step=4 and safety_factor = 0.8
#     (so a dt = 4 / 0.8 = 5 is theoretically stable)
# (2) Ensure the MassMatrix uses the scaled density

[Mesh]
  [3D]
  type = CartesianMeshGenerator
  dim = 3
  dx = '1 2 3 4 5'
  dy = 5
  dz = 5
  ix = '1 1 1 1 1'
  iy = 1
  iz = 1
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Problem]
  extra_tag_matrices = mass
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [TM_DynamicSolidMechanics0]
    type = DynamicStressDivergenceTensors
    component = 0
    use_displaced_mesh = false
    variable = disp_x
    implicit = false
  []
  [TM_DynamicSolidMechanics1]
    type = DynamicStressDivergenceTensors
    component = 1
    use_displaced_mesh = false
    variable = disp_y
    implicit = false
  []
  [TM_DynamicSolidMechanics2]
    type = DynamicStressDivergenceTensors
    component = 2
    use_displaced_mesh = false
    variable = disp_z
    implicit = false
  []
  [massmatrix_x]
    type = MassMatrix
    density = density_scaled
    matrix_tags = mass
    variable = disp_x
  []
  [massmatrix_y]
    type = MassMatrix
    density = density_scaled
    matrix_tags = mass
    variable = disp_y
  []
  [massmatrix_z]
    type = MassMatrix
    density = density_scaled
    matrix_tags = mass
    variable = disp_z
  []
[]

[BCs]
  [Pressure]
    [left]
      boundary = left
      postprocessor = -1
      displacements = 'disp_x disp_y disp_z'
    []
  []
  [right_fixed]
    type = ExplicitDirichletBC
    variable = disp_x
    boundary = right
    value = 0.0
    implicit = false
  []
  [zero_y]
    type = ExplicitDirichletBC
    variable = disp_y
    boundary = 'top bottom'
    value = 0.0
    implicit = false
  []
  [zero_z]
    type = ExplicitDirichletBC
    variable = disp_z
    boundary = 'back front'
    value = 0.0
    implicit = false
  []
[]

[Materials]
  [density_true]
    type = GenericConstantMaterial
    prop_names = density_true
    prop_values = ${density_true}
    implicit = false
  []
  [density_scaled]
    type = DensityScaling
    true_density = density_true
    scaled_density = density_scaled
    desired_time_step = 4
    safety_factor = 0.8
    output_properties = density_scaled
    outputs = exodus
    implicit = false
  []
  [Elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = ${young}
    poissons_ratio = ${poisson}
    implicit = false
  []
  [strain]
    type = ComputeSmallStrain
    implicit = false
  []
  [stress]
    type = ComputeLinearElasticStress
    implicit = false
  []

[]

[Postprocessors]
  [crit_dt_original]
    type = CriticalTimeStep
    density = density_true
    execute_on = INITIAL
  []
  [dt_possible]
    type = CriticalTimeStep
    density = density_scaled
    execute_on = INITIAL
  []
[]

[Executioner]
  type = Transient

  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = mass
    second_order_vars = 'disp_x disp_y disp_z'
    use_constant_mass = true
  []

  dt = 4
  dtmin = 4
  end_time = 12
[]


[Outputs]
  exodus = true
[]
