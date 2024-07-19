###########################################################
# This is a simple test with a time-dependent problem
# demonstrating the use of a central difference with a
# direct calculation of acceleration.
#
# Testing that the first and second time derivatives
# are calculated correctly using the Central Difference Direct
# method
###########################################################

[Problem]
    extra_tag_matrices = 'mass'
[]

[GlobalParams]
    displacements = 'disp_x disp_y'
[]

[Mesh]
    type = GeneratedMesh
    dim = 2
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    nx = 1
    ny = 1
[]

[Variables]
    [disp_x]
    []
    [disp_y]
    []
[]

[AuxVariables]
    [vel_x]
    []
    [accel_x]
    []
    [vel_y]
    []
    [accel_y]
    []
    [vel_z]
    []
[]

[AuxKernels]
    [accel_x] #All of these copy the accel and vel from the time integrator into an aux variable
        type = TestNewmarkTI
        variable = accel_x
        displacement = disp_x
        first = false
        execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
    []
    [vel_x]
        type = TestNewmarkTI
        variable = vel_x
        displacement = disp_x
        execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
    []
    [accel_y]
        type = TestNewmarkTI
        variable = accel_y
        displacement = disp_y
        first = false
        execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
    []
    [vel_y]
        type = TestNewmarkTI
        variable = vel_y
        displacement = disp_y
        execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
    []
[]

[Functions]
    [forcing_fn]
        type = PiecewiseLinear
        x = '0.0 0.1 0.2    0.3  0.4    0.5  0.6'
        y = '0.0 0.0 0.0025 0.01 0.0175 0.02 0.02'
    []
[]

[Kernels]
    [DynamicSolidMechanics]
        displacements = 'disp_x disp_y'
    []
    [massmatrix]
        type = MassMatrix
        density = 1
        matrix_tags = 'mass'
        variable = disp_x
    []
    [massmatrix_y]
        type = MassMatrix
        density = 1
        matrix_tags = 'mass'
        variable = disp_y
    []
[]

[Materials]
    [elasticity_tensor_block_one]
        type = ComputeIsotropicElasticityTensor
        youngs_modulus = 1e1
        poissons_ratio = 0.0
    []

    [strain_block]
        type = ComputeFiniteStrain # ComputeIncrementalSmallStrain
        displacements = 'disp_x disp_y'
        implicit = false
    []
    [stress_block]
        type = ComputeFiniteStrainElasticStress
    []
    [density_one]
        type = GenericConstantMaterial
        prop_names = density
        prop_values = 1
    []
    [wave_speed]
        type = WaveSpeed
        outputs = 'exodus'
        output_properties = 'wave_speed'
    []
[]

[BCs]
    # [left]
    #     type = FunctionDirichletBC
    #     variable = disp_x
    #     boundary = 'left'
    #     function = forcing_fn
    #     preset = false
    # []
    [left]
        type = DirectDirichletBC
        variable = disp_x
        value = 0
        boundary = 'left'
    []
    [right]
        type = FunctionDirichletBC
        variable = disp_x
        boundary = 'right'
        function = forcing_fn
        preset = false
    []
[]

[Executioner]
    type = Transient

    [TimeIntegrator]
        type = CentralDifferenceDirect
        solve_type = lumped
        mass_matrix_tag = 'mass'
    []

    start_time = 0.0
    num_steps = 6
    dt = 0.1
[]

[Postprocessors]
    [critical_time_step]
        type = CriticalTimeStep
    []
    [udot]
        type = ElementAverageTimeDerivative
        variable = disp_x
    []
    [udotdot]
        type = ElementAverageSecondTimeDerivative
        variable = disp_x
    []
    [u]
        type = ElementAverageValue
        variable = disp_x
    []
[]

[Outputs]
    csv = true
    exodus = true
[]
  