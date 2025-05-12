### This file contains a snippet from a Griffin input file. This file is not runnable in this form. ###

[TransportSystems]
  particle = neutron
  G = 33
  VacuumBoundary = 'top bottom radial'
  equation_type = eigenvalue
  [sn]
    scheme = DFEM-SN
    family = L2_LAGRANGE
    order = FIRST
    AQtype = Gauss-Chebyshev
    NPolar = 3
    NAzmthl = 4
    NA = 2
    sweep_type = asynchronous_parallel_sweeper
    using_array_variable = true
    collapse_scattering  = true
  []
[]

[Materials]
  [icore]
    type = MixedNeutronicsMaterial
    material_id = 1
    block = '1'
    isotopes = 'pseudo_ICORE'
  []
[]

[PowerDensity]
  power_density_variable = power
  power = 60.0
[]

[VectorPostprocessors]
  [assembly_power_2d]
    type = ExtraIDIntegralVectorPostprocessor
    variable = 'power'
    id_name = 'assembly_id'
  []
  [axial_power]
    type = ExtraIDIntegralVectorPostprocessor
    variable = 'power'
    id_name = 'plane_id'
  []
  [assembly_power_3d]
    type = ExtraIDIntegralVectorPostprocessor
    variable = 'power'
    id_name = 'assembly_id plane_id'
  []
[]
