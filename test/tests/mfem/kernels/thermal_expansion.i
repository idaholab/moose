# Thermal bending of a cantilever beam heated on its top face. The temperature and the
# displacement are solved for together in a single monolithic system.

# Material properties typical of steel.
youngs_modulus = 200e9 # Pa
poissons_ratio = 0.3
thermal_expansion_coefficient = 12e-6 # 1/K
thermal_conductivity = 50 # W/(m K)

# Temperature at which the beam is free of thermal stress (K).
reference_temperature = 293.15
# The bottom face is held at the reference temperature and the top face 100 K above it (K).
bottom_temperature = ${reference_temperature}
top_temperature = ${fparse reference_temperature + 100}

lambda = ${fparse youngs_modulus * poissons_ratio / ((1 + poissons_ratio) * (1 - 2 * poissons_ratio))}
mu = ${fparse youngs_modulus / (2 * (1 + poissons_ratio))}
# Thermal stress modulus relating a temperature change to the isotropic stress it induces in a
# fully constrained body.
beta = ${fparse thermal_expansion_coefficient * (3 * lambda + 2 * mu)}

[Mesh]
  [beam]
    type = MFEMGeneratedMeshGenerator
    dim = 3
    nx = 32
    ny = 4
    nz = 4
    xmax = 8
    ymax = 1
    zmax = 1
    elem_type = HEX
  []
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  # The thermal strain enters the stress alongside the displacement gradient, so the temperature
  # is discretized one order lower than the displacement.
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
  [H1VectorFESpace]
    type = MFEMVectorFESpace
    fec_type = H1
    fec_order = SECOND
    range_dim = 3
  []
[]

[Variables]
  [temperature]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [displacement]
    type = MFEMVariable
    fespace = H1VectorFESpace
  []
[]

[BCs]
  [cold_bottom]
    type = MFEMScalarDirichletBC
    variable = temperature
    boundary = bottom
    coefficient = ${bottom_temperature}
  []
  [hot_top]
    type = MFEMScalarDirichletBC
    variable = temperature
    boundary = top
    coefficient = ${top_temperature}
  []
  [clamped_root]
    type = MFEMVectorDirichletBC
    variable = displacement
    boundary = left
  []
[]

[FunctorMaterials]
  [steel]
    type = MFEMGenericFunctorMaterial
    prop_names = 'thermal_conductivity lambda mu'
    prop_values = '${thermal_conductivity} ${lambda} ${mu}'
  []
[]

[Kernels]
  [heat_conduction]
    type = MFEMDiffusionKernel
    variable = temperature
    coefficient = thermal_conductivity
  []
  [elasticity]
    type = MFEMLinearElasticityKernel
    variable = displacement
    lambda = lambda
    mu = mu
  []
  # Thermal strain contribution -(beta * (temperature - reference_temperature), div(v)) to the
  # momentum balance, split into a term coupling the temperature into the displacement equation
  # and a source term from the reference temperature moved to the right hand side.
  [thermal_stress]
    type = MFEMVectorDivergenceKernel
    variable = displacement
    trial_variable = temperature
    coefficient = ${fparse -beta}
    transpose = true
  []
  [thermal_stress_reference]
    type = MFEMVectorDomainLFGradKernel
    variable = displacement
    coefficient = ${fparse -beta * reference_temperature}
  []
[]

[Solvers]
  [main]
    type = MFEMMUMPS
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [centreline]
    type = MFEMVariableLineValueSampler
    variable = displacement
    start_point = '0 0.5 0.5'
    end_point = '8 0.5 0.5'
    num_points = 9
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/ThermalExpansion
  []
  [CSV]
    type = CSV
    file_base = OutputData/ThermalExpansion
  []
[]
