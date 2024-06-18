# One element test to test the central difference time integrator in 3D.
[GlobalParams]
    displacements = 'disp_x disp_y disp_z'
    volumetric_locking_correction = true
[]

[Mesh]
    [block_one]
        type = GeneratedMeshGenerator
        dim = 3
        nx = 3
        ny = 3
        nz = 3
        xmin = 4.5
        xmax = 5.5
        ymin = 4.5
        ymax = 5.5
        zmin = 0.06
        zmax = 1.06
        boundary_name_prefix = 'ball'
    []
    [block_two]
        type = GeneratedMeshGenerator
        dim = 3
        nx = 2
        ny = 2
        nz = 2
        xmin = 0.0
        xmax = 10
        ymin = 0.0
        ymax = 10
        zmin = -2
        zmax = 0
        boundary_name_prefix = 'base'
        boundary_id_offset = 10
    []
    [block_one_id]
        type = SubdomainIDGenerator
        input = block_one
        subdomain_id = 1
    []
    [block_two_id]
        type = SubdomainIDGenerator
        input = block_two
        subdomain_id = 2
    []
    [combine]
        type = MeshCollectionGenerator
        inputs = ' block_one_id block_two_id'
    []
[]

[AuxVariables]
    [penetration]
    []
[]

[AuxKernels]
    [penetration]
        type = PenetrationAux
        variable = penetration
        boundary = ball_back
        paired_boundary = base_front
        quantity = distance
    []
[]

[Variables]
    [disp_x]
    []
    [disp_y]
    []
    [disp_z]
    []
[]

[AuxVariables]
    [gap_rate]
    []
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
    [accel_z]
    []
    [stress_zz]
        family = MONOMIAL
        order = CONSTANT
    []
    [strain_zz]
        family = MONOMIAL
        order = CONSTANT
    []
    [kinetic_energy_one]
        order = CONSTANT
        family = MONOMIAL
    []
    [elastic_energy_one]
        order = CONSTANT
        family = MONOMIAL
    []
    [kinetic_energy_two]
        order = CONSTANT
        family = MONOMIAL
    []
    [elastic_energy_two]
        order = CONSTANT
        family = MONOMIAL
    []
[]

[AuxKernels]
    [stress_zz]
        type = RankTwoAux
        rank_two_tensor = stress
        index_i = 2
        index_j = 2
        variable = stress_zz
        execute_on = 'TIMESTEP_END'
    []
    [strain_zz]
        type = RankTwoAux
        rank_two_tensor = mechanical_strain
        index_i = 2
        index_j = 2
        variable = strain_zz
    []
    [accel_x]
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
        displacement = disp_x
        execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
    []
    [accel_z]
        type = TestNewmarkTI
        variable = accel_z
        displacement = disp_z
        first = false
        execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
    []
    [vel_z]
        type = TestNewmarkTI
        variable = vel_z
        displacement = disp_z
        execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
    []
    [kinetic_energy_one]
        type = KineticEnergyAux
        block = '1'
        variable = kinetic_energy_one
        newmark_velocity_x = vel_x
        newmark_velocity_y = vel_y
        newmark_velocity_z = vel_z
        density = density
    []
    [elastic_energy_one]
        type = ElasticEnergyAux
        variable = elastic_energy_one
        block = '1'
    []
    [kinetic_energy_two]
        type = KineticEnergyAux
        block = '2'
        variable = kinetic_energy_two
        newmark_velocity_x = vel_x
        newmark_velocity_y = vel_y
        newmark_velocity_z = vel_z
        density = density
    []
    [elastic_energy_two]
        type = ElasticEnergyAux
        variable = elastic_energy_two
        block = '2'
    []
[]

[Kernels]
    [DynamicTensorMechanics]
        displacements = 'disp_x disp_y disp_z'
        stiffness_damping_coefficient = 1.0e-3
        generate_output = 'stress_zz strain_zz'
    []
    [inertia_x]
        type = InertialForce
        variable = disp_x
    []
    [inertia_y]
        type = InertialForce
        variable = disp_y
    []
    [inertia_z]
        type = InertialForce
        variable = disp_z
    []
    [gravity]
        type = Gravity
        variable = disp_z
        value = -981
    []
[]

[BCs]
    [x_front]
        type = DirichletBC
        variable = disp_x
        boundary = 'ball_front'
        preset = false
        value = 0.0
    []
    [y_front]
        type = DirichletBC
        variable = disp_y
        boundary = 'ball_front'
        preset = false
        value = 0.0
    []
    [x_fixed]
        type = DirichletBC
        variable = disp_x
        boundary = 'base_back'
        preset = false
        value = 0.0
    []
    [y_fixed]
        type = DirichletBC
        variable = disp_y
        boundary = 'base_back'
        preset = false
        value = 0.0
    []
    [z_fixed]
        type = DirichletBC
        variable = disp_z
        boundary = 'base_back'
        preset = false
        value = 0.0
    []
    [z_fixed_front]
        type = DirichletBC
        variable = disp_z
        boundary = 'base_front'
        preset = false
        value = 0.0
    []
[]

[ExplicitDynamicsContact]
    [my_contact]
        model = frictionless_balance
        primary = base_front
        secondary = ball_back
        vel_x = 'vel_x'
        vel_y = 'vel_y'
        vel_z = 'vel_z'
    []
[]

[Materials]
    [elasticity_tensor_block_one]
        type = ComputeIsotropicElasticityTensor
        youngs_modulus = 1e6
        poissons_ratio = 0.0
        block = 1
        outputs = 'exodus'
        output_properties = __all__
    []
    [elasticity_tensor_block_two]
        type = ComputeIsotropicElasticityTensor
        youngs_modulus = 1e10
        poissons_ratio = 0.0
        block = 2
        outputs = 'exodus'
        output_properties = __all__
    []
    [strain_block]
        type = ComputeFiniteStrain # ComputeIncrementalSmallStrain
        displacements = 'disp_x disp_y disp_z'
        implicit = false
    []
    [stress_block]
        type = ComputeFiniteStrainElasticStress
    []
    [density_one]
        type = GenericConstantMaterial
        prop_names = density
        prop_values = 1e1
        outputs = 'exodus'
        output_properties = 'density'
        block = '1'
    []
    [density_two]
        type = GenericConstantMaterial
        prop_names = density
        prop_values = 1e6
        outputs = 'exodus'
        output_properties = 'density'
        block = '2'
    []
    [wave_speed]
        type = WaveSpeed
        outputs = 'exodus'
        output_properties = 'wave_speed'
    []
[]

[Executioner]
    type = Transient

    end_time = 0.02
    dt = 2e-4
    timestep_tolerance = 1e-6

    [TimeIntegrator]
        type = CentralDifference
        solve_type = lumped
    []
[]

[Outputs]
    interval = 2
    exodus = true
    csv = true
    execute_on = 'TIMESTEP_END'
    file_base = highvel_out
[]

[Postprocessors]
    [accel_58z]
        type = NodalVariableValue
        nodeid = 1
        variable = accel_z
    []
    [vel_58z]
        type = NodalVariableValue
        nodeid = 1
        variable = vel_z
    []
    [critical_time_step]
        type = CriticalTimeStep
    []
    [contact_pressure_max]
        type = NodalExtremeValue
        variable = contact_pressure
        block = '1 2'
        value_type = max
    []
    [penetration_max]
        type = NodalExtremeValue
        variable = penetration
        block = '1 2'
        value_type = max
    []
    [total_kinetic_energy_one]
        type = ElementIntegralVariablePostprocessor
        variable = kinetic_energy_one
        block = '1'
    []
    [total_elastic_energy_one]
        type = ElementIntegralVariablePostprocessor
        variable = elastic_energy_one
        block = '1'
    []
    [total_kinetic_energy_two]
        type = ElementIntegralVariablePostprocessor
        variable = kinetic_energy_two
        block = '2'
    []
    [total_elastic_energy_two]
        type = ElementIntegralVariablePostprocessor
        variable = elastic_energy_two
        block = '2'
    []
[]

