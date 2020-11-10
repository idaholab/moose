# The purpose of this script is to create the input files named below
# The input files are conceptually fairly simple, but are quite long and repetitive because of the large number of species involved, so this python file helps prevent typos

porous_flow_filename = "porous_flow.i"
injection_bh_filename = "injection.bh"
production_bh_filename = "production.bh"
aquifer_geochem_filename = "aquifer_geochemistry.i"
exchanger_filename = "exchanger.i"

inject_rate = 0.02 # injection rate in kg/s/m

# names of the species in the porous-flow simulations (no brackets, minus signs, etc, so they can be used in Parsed quantities)
var_name = ["H", "Cl", "SO4", "HCO3", "SiO2aq", "Al", "Ca", "Mg", "Fe", "K", "Na", "Sr", "F", "BOH", "Br", "Ba", "Li", "NO3", "O2aq", "H2O"]
# names of the geochemical species corresponding to var_name
geochem_vars = ["H+", "Cl-", "SO4--", "HCO3-", "SiO2(aq)", "Al+++", "Ca++", "Mg++", "Fe++", "K+", "Na+", "Sr++", "F-", "B(OH)3", "Br-", "Ba++", "Li+", "NO3-", "O2(aq)", "H2O"]

# these are the initial mass fractions of the var_name species.  The numbers are obtained by recording the transported mass fractions of each species as postprocessors: run one time-step of aquifer_geochemistry.i to find the numbers
ic = [-2.952985071156e-06, 0.04870664551708, 0.0060359986852517, 5.0897287594019e-05, 3.0246609868421e-05, 3.268028901929e-08, 0.00082159428184586, 1.8546347062146e-05, 4.3291908204093e-05, 6.8434768308898e-05, 0.033298053919671, 1.2771866652177e-05, 5.5648860174073e-06, 0.0003758574621917, 9.0315286107068e-05, 1.5637460875161e-07, 8.3017067912701e-05, 0.00010958455036169, -7.0806852373351e-05, 0.91032275033842]

# mol weight of each species
mol_weight = [1.0079, 35.453, 96.0576, 61.0171, 60.0843, 26.9815, 40.08, 24.305, 55.847, 39.0983, 22.9898, 87.62, 18.9984, 61.8329, 79.904, 137.33, 6.941, 62.0049, 31.9988, 18.01801802]

# all the minerals in the system
all_minerals = ["Siderite", "Pyrrhotite", "Dolomite", "Illite", "Anhydrite", "Calcite", "Quartz", "K-feldspar", "Kaolinite", "Barite", "Celestite", "Fluorite", "Albite", "Chalcedony", "Goethite"]

# this impacts the Mesh created and the borehole Dirac points
# resolution = 1 means lowest resolution
# resolution = 2 is higher
resolution = 1

def write_header(f):
    # all file written have this preprended to them to alert a reader that it wasn't written by hand
    f.write("#########################################\n")
    f.write("#                                       #\n")
    f.write("# File written by create_input_files.py #\n")
    f.write("#                                       #\n")
    f.write("#########################################\n")

def write_executioner(f):
    # executioner used
    f.write("[Executioner]\n  type = Transient\n  solve_type = Newton\n  end_time = 7.76E6 # 90 days\n")
    f.write("  [./TimeStepper]\n    type = FunctionDT\n    function = 'min(3E4, max(1E4, 0.2 * t))'\n  [../]\n[]\n")

def nxnynz(res):
    # return (nx, ny, nz) for the specified resolution.  This is used in creating the mesh, and in the nodal_volume AuxKernel (until PR #15691 is merged)
    # Note that if you make another "res" option that has a high nz, you'll also have to increase the number of z coords in injection_by_filename and production_bh_filename
    nx = 15
    ny = 4
    nz = 5
    if (res == 2):
        nx = 30
        ny = 8
        nz = 10
    return (nx, ny, nz)

def write_mesh(f, res):
    # 3D mesh used
    f.write("[Mesh]\n  [gen]\n    type = GeneratedMeshGenerator\n    dim = 3\n")
    f.write("    xmin = -75\n    xmax = 75\n    ymin = 0\n    ymax = 40\n    zmin = -25\n    zmax = 25\n")
    nx, ny, nz = nxnynz(res)
    f.write("    nx = " + str(nx) + "\n    ny = " + str(ny) + "\n    nz = " + str(nz) + "\n")
    f.write("  []\n")
    f.write("  [./aquifer]\n    type = ParsedSubdomainMeshGenerator\n    input = gen\n    block_id = 1\n    block_name = aquifer\n    combinatorial_geometry = 'z >= -5 & z <= 5'\n  [../]\n")
    # res = 1 means low-resolution
    injection_nodesets = "'-25 0 -5; -25 0 5'"
    production_nodesets = "'25 0 -5; 25 0 5'"
    if (res == 2):
        injection_nodesets = "'-25 0 -5; -25 0 0; -25 0 5'"
        production_nodesets = "'25 0 -5; 25 0 0; 25 0 5'"
    f.write("  [./injection_nodes]\n    input = aquifer\n    type = ExtraNodesetGenerator\n    new_boundary = injection_nodes\n    coord = " + injection_nodesets + "\n  [../]\n")
    f.write("  [./production_nodes]\n    input = injection_nodes\n    type = ExtraNodesetGenerator\n    new_boundary = production_nodes\n    coord = " + production_nodesets + "\n  [../]\n")
    f.write("[]\n")


import os
import sys


sys.stdout.write("Outputting injection borehole specification to " + injection_bh_filename + "\n")
f = open(injection_bh_filename, "w")
write_header(f)
for z in [-5, -3, -1, 1, 3, 5]:
    f.write("1.0 -25.0 0.0 " + str(z) + "\n")
f.close()

sys.stdout.write("Outputting production borehole specification to " + production_bh_filename + "\n")
f = open(production_bh_filename, "w")
write_header(f)
for z in [-5, -3, -1, 1, 3, 5]:
    f.write("1.0 25.0 0.0 " + str(z) + "\n")
f.close()

sys.stdout.write("Outputting porous-flow input file to " + porous_flow_filename + "\n")
f = open(porous_flow_filename, "w")
write_header(f)

f.write("# PorousFlow simulation of injection and production in a simplified GeoTES aquifer\n# Much of this file is standard porous-flow stuff.  The unusual aspects are:\n# - transfer of the rates of changes of each species (kg.s) to the aquifer_geochemistry.i simulation.  This is achieved by saving these changes from the PorousFlowMassTimeDerivative residuals\n# - transfer of the temperature field to the aquifer_geochemistry.i simulation\n# Interesting behaviour can be simulated by this file without its 'parent' simulation, exchanger.i.  exchanger.i provides mass-fractions injected via the injection_rate_massfrac_* variables, but since these are more-or-less constant throughout the duration of the exchanger.i simulation, the initial_conditions specified below may be used.  Similar, exchanger.i provides injection_temperature, but that is also constant.\n")

f.write("injection_rate = " + str(-inject_rate) + " # kg/s/m, negative because injection as a source\n")
f.write("production_rate = " + str(inject_rate) + " # kg/s/m, this is about the maximum that can be sustained by the aquifer, with its fairly low permeability, without porepressure becoming negative\n")

write_mesh(f, resolution)

f.write("\n[GlobalParams]\n  PorousFlowDictator = dictator\n  gravity = '0 0 -10'\n[]\n")

f.write("[BCs]\n  [./injection_temperature]\n    type = MatchedValueBC\n    variable = temperature\n    v = injection_temperature\n    boundary = injection_nodes\n  [../]\n[]\n")

f.write("[Modules]\n  [./FluidProperties]\n    [./the_simple_fluid]\n      type = SimpleFluidProperties\n      thermal_expansion = 0\n      bulk_modulus = 2E9\n      viscosity = 1E-3\n      density0 = 1000\n      cv = 4000.0\n      cp = 4000.0\n    [../]\n  [../]\n[]\n\n")

f.write("[Materials]\n  [./temperature]\n    type = PorousFlowTemperature\n    temperature = temperature\n  [../]\n  [./fluid_props]\n    type = PorousFlowSingleComponentFluid\n    fp = the_simple_fluid\n    temperature_unit = Celsius\n    phase = 0\n  [../]\n  [./saturation]\n    type = PorousFlow1PhaseP\n    porepressure = porepressure\n    capillary_pressure = capillary_pressure\n  [../]\n  [./relperm]\n    type = PorousFlowRelativePermeabilityConst\n    phase = 0\n  [../]\n  [./porosity_caps]\n    type = PorousFlowPorosityConst # this simulation has no porosity changes from dissolution\n    block = 0\n    porosity = 0.01\n  [../]\n  [./porosity_aquifer]\n    type = PorousFlowPorosityConst # this simulation has no porosity changes from dissolution\n    block = aquifer\n    porosity = 0.063\n  [../]\n  [./permeability_caps]\n    type = PorousFlowPermeabilityConst\n    block = 0\n    permeability = '1E-18 0 0   0 1E-18 0   0 0 1E-18'\n  [../]\n  [./permeability_aquifer]\n    type = PorousFlowPermeabilityConst\n    block = aquifer\n    permeability = '1.7E-15 0 0   0 1.7E-15 0   0 0 4.1E-16'\n  [../]\n  [./rock_heat]\n    type = PorousFlowMatrixInternalEnergy\n    density = 2500.0\n    specific_heat_capacity = 1200.0\n  [../]\n")
f.write("  [./mass_frac]\n    type = PorousFlowMassFraction\n    mass_fraction_vars = '")
for i in range(19):
    f.write("f_" + var_name[i] + " ")
f.write("'\n  [../]\n")
f.write("[]\n")


f.write("[Preconditioning]\n  active = typically_efficient\n  [./typically_efficient]\n    type = SMP\n    full = true\n    petsc_options_iname = '-pc_type -pc_hypre_type'\n    petsc_options_value = ' hypre    boomeramg'\n  [../]\n  [./strong]\n    type = SMP\n    full = true\n    petsc_options = '-ksp_diagonal_scale -ksp_diagonal_scale_fix'\n    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'\n    petsc_options_value = ' asm      ilu           NONZERO                   2'\n  [../]\n  [./probably_too_strong]\n    type = SMP\n    full = true\n    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'\n    petsc_options_value = ' lu       mumps'\n  [../]\n[]\n")

write_executioner(f)

f.write("[Outputs]\n  exodus = true\n[]\n")


f.write("[Variables]\n")
for i in range(19):
    f.write("  [./f_" + var_name[i] + "]\n    initial_condition = " + str(ic[i]) + "\n  [../]\n")
f.write("  [./porepressure]\n    initial_condition = 30E6\n  [../]\n  [./temperature]\n    initial_condition = 92\n    scaling = 1E-6 # fluid enthalpy is roughly 1E6\n  [../]\n[]\n")


f.write("\n")
f.write("[DiracKernels]\n")
for i in range(19):
    f.write("  [./inject_" + var_name[i] + "]\n    type = PorousFlowPolyLineSink\n    SumQuantityUO = injected_mass\n    fluxes = ${injection_rate}\n    p_or_t_vals = 0.0\n    multiplying_var = injection_rate_massfrac_" + var_name[i] + "\n    point_file = " + injection_bh_filename + "\n    variable = f_" + var_name[i] + "\n  [../]\n")
i = 19
f.write("  [./inject_" + var_name[i] + "]\n    type = PorousFlowPolyLineSink\n    SumQuantityUO = injected_mass\n    fluxes = ${injection_rate}\n    p_or_t_vals = 0.0\n    multiplying_var = injection_rate_massfrac_" + var_name[i] + "\n    point_file = " + injection_bh_filename + "\n    variable = porepressure\n  [../]\n")

f.write("\n")
for i in range(19):
    f.write("  [./produce_" + var_name[i] + "]\n    type = PorousFlowPolyLineSink\n    SumQuantityUO = produced_mass_" + var_name[i] + "\n    fluxes = ${production_rate}\n    p_or_t_vals = 0.0\n    mass_fraction_component = " + str(i) + "\n    point_file = " + production_bh_filename + "\n    variable = f_" + var_name[i] + "\n  [../]\n")
i = 19
f.write("  [./produce_" + var_name[i] + "]\n    type = PorousFlowPolyLineSink\n    SumQuantityUO = produced_mass_" + var_name[i] + "\n    fluxes = ${production_rate}\n    p_or_t_vals = 0.0\n    mass_fraction_component = " + str(i) + "\n    point_file = " + production_bh_filename + "\n    variable = porepressure\n  [../]\n")
f.write("  [./produce_heat]\n    type = PorousFlowPolyLineSink\n    SumQuantityUO = produced_heat\n    fluxes = ${production_rate}\n    p_or_t_vals = 0.0\n    use_enthalpy = true\n    point_file = " + production_bh_filename + "\n    variable = temperature\n  [../]\n")
f.write("[]\n")

f.write("\n")
f.write("[UserObjects]\n")
f.write("  [./injected_mass]\n    type = PorousFlowSumQuantity\n  [../]\n")
for i in range(20):
    f.write("  [./produced_mass_" + var_name[i] + "]\n    type = PorousFlowSumQuantity\n  [../]\n")
f.write("  [./produced_heat]\n    type = PorousFlowSumQuantity\n  [../]\n")
f.write("\n")
f.write("  [./capillary_pressure]\n    type = PorousFlowCapillaryPressureConst\n  [../]\n")
f.write("\n")
f.write("  [./dictator]\n    type = PorousFlowDictator\n    porous_flow_vars = 'porepressure temperature")
for i in range(19):
    f.write(" f_" + var_name[i])
f.write("'\n    number_fluid_phases = 1\n    number_fluid_components = " + str(len(var_name)) + "\n  [../]\n")
f.write("[]")

f.write("\n")
f.write("[Postprocessors]\n")
f.write("  [./dt]\n    type = TimestepSize\n    execute_on = TIMESTEP_BEGIN\n  [../]\n  [./tot_kg_injected_this_timestep]\n    type = PorousFlowPlotQuantity\n    uo = injected_mass\n  [../]\n")
for i in range(20):
    f.write("  [./kg_" + var_name[i] + "_produced_this_timestep]\n    type = PorousFlowPlotQuantity\n    uo = produced_mass_" + var_name[i] + "\n  [../]\n")
for i in range(20):
    f.write("  [./mole_rate_" + var_name[i] + "_produced]\n    type = FunctionValuePostprocessor\n    function = moles_" + var_name[i] + "\n  [../]\n")
f.write("  [./heat_joules_extracted_this_timestep]\n    type = PorousFlowPlotQuantity\n    uo = produced_heat\n  [../]\n  [./production_temperature]\n    type = AverageNodalVariableValue\n    boundary = production_nodes\n    variable = temperature\n  [../]\n")
f.write("[]\n")

f.write("\n")
f.write("[Functions]\n")
for i in range(20):
    f.write("  [./moles_" + var_name[i] + "]\n    type = ParsedFunction\n    vars = 'kg_" + var_name[i] + " dt'\n    vals = 'kg_" + var_name[i] + "_produced_this_timestep dt'\n    value = 'kg_" + var_name[i] + " * 1000 / " + str(mol_weight[i]) + " / dt'\n  [../]\n")
f.write("[]\n")

f.write("\n")
f.write("[Kernels]\n")
for i in range(19):
    f.write("  [./advective_flux_" + var_name[i] + "]\n    type = PorousFlowAdvectiveFlux\n    fluid_component = " + str(i) + "\n    variable = f_" + var_name[i] + "\n  [../]\n")
i = 19
f.write("  [./advective_flux_" + var_name[i] + "]\n    type = PorousFlowAdvectiveFlux\n    fluid_component = " + str(i) + "\n    variable = porepressure\n  [../]\n")
for i in range(19):
    f.write("  [./time_deriv_" + var_name[i] + "]\n    type = PorousFlowMassTimeDerivative\n    fluid_component = " + str(i) + "\n    save_in = rate_" + str(var_name[i]) + " # change in kg at every node / dt\n    variable = f_" + var_name[i] + "\n  [../]\n")
i = 19
f.write("  [./time_deriv_" + var_name[i] + "]\n    type = PorousFlowMassTimeDerivative\n    fluid_component = " + str(i) + "\n    save_in = rate_" + str(var_name[i]) + " # change in kg at every node / dt\n    variable = porepressure\n  [../]\n")
f.write("  [./temperature_advection]\n    type = PorousFlowHeatAdvection\n    variable = temperature\n  [../]\n  [./temperature_time_deriv]\n    type = PorousFlowEnergyTimeDerivative\n    variable = temperature\n  [../]\n")
f.write("[]\n")


f.write("\n")
f.write("[AuxVariables]\n  [./injection_temperature]\n    initial_condition = 92\n  [../]\n")
for i in range(20):
    f.write("  [./injection_rate_massfrac_" + var_name[i] + "]\n    initial_condition = " + str(ic[i]) + "\n  [../]\n")
for i in range(20):
    f.write("  [./rate_" + var_name[i] + "]\n  [../]\n")
f.write("[]\n")

f.write("\n")
f.write("[MultiApps]\n  [./react]\n    type = TransientMultiApp\n    input_files = aquifer_geochemistry.i\n    clone_master_mesh = true\n    execute_on = 'timestep_end'\n  [../]\n[]\n")

f.write("[Transfers]\n")
f.write("  [./changes_due_to_flow]\n    type = MultiAppCopyTransfer\n    direction = to_multiapp\n    source_variable = '")
for i in range(20):
    f.write("rate_" + var_name[i] + " ")
f.write("temperature'\n")
f.write("    variable = '")
for i in range(20):
    f.write("pf_rate_" + var_name[i] + " ")
f.write("temperature'\n")
f.write("    multi_app = react\n  [../]\n")
f.write("  [./massfrac_from_geochem]\n    type = MultiAppCopyTransfer\n    direction = from_multiapp\n    source_variable = '")
for i in range(19):
    f.write("massfrac_" + var_name[i] + " ")
f.write("'\n")
f.write("    variable = '")
for i in range(19):
    f.write("f_" + var_name[i] + " ")
f.write("'\n    multi_app = react\n  [../]\n[]\n")

f.close()

sys.stdout.write("Outputting porous-flow input file to " + aquifer_geochem_filename + "\n")
f = open(aquifer_geochem_filename, "w")
write_header(f)

f.write("# Simulates geochemistry in the aquifer.  This input file may be run in standalone fashion but it does not do anything of interest.  To simulate something interesting, run the " + porous_flow_filename + " simulation which couples to this input file using MultiApps.\n")
f.write("# This file receives")
for i in range(20):
    f.write(" pf_rate_" + var_name[i])
f.write(" and temperature as AuxVariables from " + porous_flow_filename + "\n")
f.write("# The pf_rate quantities are kg/s changes of fluid-component mass at each node, but the geochemistry module expects rates-of-changes of moles at every node.  Secondly, since this input file considers just 1 litre of aqueous solution at every node, the nodal_void_volume is used to convert pf_rate_* into rate_*_per_1l, which is measured in mol/s/1_litre_of_aqueous_solution.\n")
f.write("# This file sends")
for i in range(19):
    f.write(" massfrac_" + var_name[i])
f.write(" to " + porous_flow_filename + ".  These are computed from the corresponding transported_* quantities.\n")

f.write("\n")
f.write("[UserObjects]\n  [./definition]\n    type = GeochemicalModelDefinition\n    database_file = '../../../../geochemistry/database/moose_geochemdb.json'\n    basis_species = 'H2O H+ Cl- SO4-- HCO3- SiO2(aq) Al+++ Ca++ Mg++ Fe++ K+ Na+ Sr++ F- B(OH)3 Br- Ba++ Li+ NO3- O2(aq)'\n    equilibrium_minerals = '" + " ".join(all_minerals) + "'\n  [../]\n  [./nodal_void_volume_uo]\n    type = NodalVoidVolume\n    porosity = porosity\n    execute_on = 'initial timestep_end' # initial means this is evaluated properly for the first timestep\n  [../]\n[]\n")

f.write("\n")
f.write("[SpatialReactionSolver]\n  model_definition = definition\n  geochemistry_reactor_name = reactor\n")
f.write("  charge_balance_species = 'Cl-'\n")
f.write("  swap_out_of_basis = 'NO3- H+         Fe++       Ba++   SiO2(aq) Mg++     O2(aq)   Al+++   K+     Ca++      HCO3-'\n  swap_into_basis = '  NH3  Pyrrhotite K-feldspar Barite Quartz   Dolomite Siderite Calcite Illite Anhydrite Kaolinite'\n")
f.write("# ASSUME that 1 litre of solution contains:\n")
f.write("  constraint_species = 'H2O        Quartz     Calcite   K-feldspar Siderite  Dolomite  Anhydrite Pyrrhotite Illite    Kaolinite  Barite       Na+       Cl-       SO4--       Li+         B(OH)3      Br-         F-         Sr++        NH3'\n")
f.write("  constraint_value = '  0.99778351 322.177447 12.111108 6.8269499  6.2844304 2.8670301 1.1912027 0.51474767 0.3732507 0.20903322 0.0001865889 1.5876606 1.5059455 0.046792579 0.013110503 0.006663119 0.001238987 0.00032108 0.000159781 0.001937302'\n")
f.write("  constraint_meaning = 'kg_solvent_water moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species'\n")
f.write("  prevent_precipitation = 'Fluorite Albite Goethite'\n")
f.write("  initial_temperature = 92\n")
f.write("  temperature = temperature\n")
f.write("  source_species_names = 'H+ Cl- SO4-- HCO3- SiO2(aq) Al+++ Ca++ Mg++ Fe++ K+ Na+ Sr++ F- B(OH)3 Br- Ba++ Li+ NO3- O2(aq) H2O'\n")
f.write("  source_species_rates = '")
for i in range(20):
    f.write(" rate_" + var_name[i] + "_per_1l")
f.write("'\n")
f.write("  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping\n")
f.write("  execute_console_output_on = '' # only CSV and exodus output for this simulation\n")
f.write("  add_aux_molal = false # save some memory and reduce variables in output exodus\n")
f.write("  add_aux_mg_per_kg = false # save some memory and reduce variables in output exodus\n")
f.write("  add_aux_free_mg = false # save some memory and reduce variables in output exodus\n")
f.write("  add_aux_activity = false # save some memory and reduce variables in output exodus\n")
f.write("  add_aux_bulk_moles = false # save some memory and reduce variables in output exodus\n")
f.write("  adaptive_timestepping = true\n")
f.write("[]\n")

f.write("\n")
write_mesh(f, resolution)

f.write("[GlobalParams]\n  point = '-25 0 0'\n  reactor = reactor\n[]\n")

write_executioner(f)

f.write("[AuxVariables]\n  [./temperature]\n    initial_condition = 92.0\n  [../]\n  [./porosity]\n    initial_condition = 0.1\n  [../]\n  [./nodal_void_volume]\n  [../]\n  [./free_cm3_Kfeldspar] # necessary because of the minus sign in K-feldspar which does not parse correctly in the porosity AuxKernel\n  [../]\n")
for i in range(20):
    f.write("  [./pf_rate_" + var_name[i] + "] # change in " + var_name[i] + " mass (kg/s) at each node provided by the porous-flow simulation\n  [../]\n")
for i in range(20):
    f.write("  [./rate_" + var_name[i] + "_per_1l]\n  [../]\n")
for i in range(20):
    f.write("  [./transported_" + var_name[i] + "]\n  [../]\n")
f.write("  [./transported_mass]\n  [../]\n")
for i in range(20):
    f.write("  [./massfrac_" + var_name[i] + "]\n  [../]\n")
f.write("[]\n")

f.write("\n")
f.write("[AuxKernels]\n")
f.write("  [./free_cm3_Kfeldspar]\n    type = GeochemistryQuantityAux\n    variable = free_cm3_Kfeldspar\n    species = 'K-feldspar'\n    quantity = free_cm3\n    execute_on = 'timestep_end'\n  [../]\n")
f.write("  [./porosity_auxk]\n    type = ParsedAux\n    args = 'free_cm3_Siderite free_cm3_Pyrrhotite free_cm3_Dolomite free_cm3_Illite free_cm3_Anhydrite free_cm3_Calcite free_cm3_Quartz free_cm3_Kfeldspar free_cm3_Kaolinite free_cm3_Barite free_cm3_Celestite free_cm3_Fluorite free_cm3_Albite free_cm3_Chalcedony free_cm3_Goethite'\n    function = '1000.0 / (1000.0 + free_cm3_Siderite + free_cm3_Pyrrhotite + free_cm3_Dolomite + free_cm3_Illite + free_cm3_Anhydrite + free_cm3_Calcite + free_cm3_Quartz + free_cm3_Kfeldspar + free_cm3_Kaolinite + free_cm3_Barite + free_cm3_Celestite + free_cm3_Fluorite + free_cm3_Albite + free_cm3_Chalcedony + free_cm3_Goethite)'\n    variable = porosity\n    execute_on = 'timestep_end'\n  [../]\n")
f.write("  [./nodal_void_volume_auxk]\n    type = NodalVoidVolumeAux\n    variable = nodal_void_volume\n    nodal_void_volume_uo = nodal_void_volume_uo\n    execute_on = 'initial timestep_end' # initial to ensure it is properly evaluated for the first timestep\n  [../]\n")
for i in range(20):
    f.write("  [./rate_" + var_name[i] + "_per_1l_auxk]\n")
    f.write("    type = ParsedAux\n")
    f.write("    args = 'pf_rate_" + var_name[i] + " nodal_void_volume'\n")
    f.write("    variable = rate_" + var_name[i] + "_per_1l\n")
    f.write("    function = 'pf_rate_" + var_name[i] + " / " + str(mol_weight[i]) + " / nodal_void_volume'\n")
    f.write("    execute_on = 'timestep_end'\n  [../]\n")
for i in range(20):
    f.write("  [./transported_" + var_name[i] + "_auxk]\n")
    f.write("    type = GeochemistryQuantityAux\n")
    f.write("    variable = transported_" + var_name[i] + "\n")
    f.write("    species = '" + geochem_vars[i] + "'\n")
    f.write("    quantity = transported_moles_in_original_basis\n    execute_on = 'timestep_end'\n  [../]\n")
f.write("  [./transported_mass_auxk]\n    type = ParsedAux\n    args = '")
for i in range(20):
    f.write(" transported_" + var_name[i])
f.write("'\n")
f.write("    variable = transported_mass\n")
f.write("    function = 'transported_" + var_name[0] + " * " + str(mol_weight[0]))
for i in range(1, 20):
    f.write(" + transported_" + var_name[i] + " * " + str(mol_weight[i]))
f.write("'\n")
f.write("    execute_on = 'timestep_end'\n  [../]\n")
for i in range(20):
    f.write("  [./massfrac_" + var_name[i] + "_auxk]\n    type = ParsedAux\n")
    f.write("    args = 'transported_" + var_name[i] + " transported_mass'\n")
    f.write("    variable = massfrac_" + var_name[i] + "\n")
    f.write("    function = 'transported_" + var_name[i] + " * " + str(mol_weight[i]) + " / transported_mass'\n")
    f.write("    execute_on = 'timestep_end'\n")
    f.write("  [../]\n")
f.write("[]\n")

f.write("\n")
f.write("[Postprocessors]\n")
f.write("  [./memory]\n    type = MemoryUsage\n    outputs = 'console'\n  [../]\n")
f.write("  [./porosity]\n    type = PointValue\n    variable = porosity\n  [../]\n  [./solution_temperature]\n    type = PointValue\n    variable = solution_temperature\n  [../]\n")
for i in range(20):
    f.write("  [./massfrac_" + var_name[i] + "]\n    type = PointValue\n    variable = massfrac_" + var_name[i] + "\n  [../]\n")
for mineral in all_minerals:
    f.write("  [./free_cm3_" + mineral + "]\n    type = PointValue\n    variable = free_cm3_" + mineral + "\n  [../]\n")
f.write("[]\n")

f.write("[Outputs]\n  exodus = true\n  csv = true\n[]\n")
f.close()

sys.stdout.write("Outputting heat-exchanger input file to " + exchanger_filename + "\n")
f = open(exchanger_filename, "w")
write_header(f)

f.write("# Model of the heat-exchanger\n")
f.write("# The input fluid to the heat exchanger is determined by AuxVariables called production_temperature")
for i in range(20):
    f.write(", production_rate_" + var_name[i])
f.write(".  These come from Postprocessors in the " + porous_flow_filename + " simulation that measure the fluid composition at the production well.\n")
f.write("# Given the input fluid, the exchanger cools/heats the fluid, removing any precipitates, and injects fluid back to " + porous_flow_filename + " at temperature output_temperature and composition given by massfrac_H, etc.\n")

f.write("\n")
f.write("[UserObjects]\n  [./definition]\n    type = GeochemicalModelDefinition\n    database_file = '../../../../geochemistry/database/moose_geochemdb.json'\n    basis_species = 'H2O H+ Cl- SO4-- HCO3- SiO2(aq) Al+++ Ca++ Mg++ Fe++ K+ Na+ Sr++ F- B(OH)3 Br- Ba++ Li+ NO3- O2(aq)'\n    equilibrium_minerals = '" + " ".join(all_minerals) + "'\n  [../]\n[]\n")

f.write("\n")
f.write("[TimeDependentReactionSolver]\n  model_definition = definition\n  include_moose_solve = false\n  geochemistry_reactor_name = reactor\n")
f.write("  swap_out_of_basis = 'NO3- O2(aq)'\n  swap_into_basis = '  NH3  HS-'\n")
f.write("  charge_balance_species = 'Cl-'\n")
f.write("# initial conditions are unimportant because in exchanger mode all existing fluid is flushed from the system before adding the produced water\n")
f.write("  constraint_species = 'H2O H+ Cl- SO4-- HCO3- SiO2(aq) Al+++ Ca++ Mg++ Fe++ K+ Na+ Sr++ F- B(OH)3 Br- Ba++ Li+ NH3 HS-'\n")
f.write("  constraint_value = '1.0 1E-6 1E-6 1E-18 1E-18 1E-18    1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18 1E-18'\n")
f.write("  constraint_meaning = 'kg_solvent_water moles_bulk_species moles_bulk_species free_molality free_molality free_molality free_molality free_molality free_molality free_molality free_molality free_molality free_molality free_molality free_molality free_molality free_molality free_molality free_molality free_molality'\n")
f.write("  prevent_precipitation = 'Fluorite Albite Goethite'\n")
f.write("  initial_temperature = 92\n")
f.write("  mode = 4\n")
f.write("  temperature = ramp_temperature # ramp up to 160degC over ~1 day so that aquifer geochemistry simulation can easily converge\n")
f.write("  cold_temperature = 92\n")
f.write("  heating_increments = 10\n")
f.write("  source_species_names = '")
for i in range(20):
    f.write(" " + geochem_vars[i])
f.write("'\n")
f.write("  source_species_rates = '")
for i in range(20):
    f.write(" production_rate_" + var_name[i])
f.write("'\n")
f.write("  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping\n")
f.write("[]\n")

f.write("\n")
f.write("[GlobalParams]\n  point = '0 0 0'\n  reactor = reactor\n[]\n")

f.write("\n")
f.write("[AuxVariables]\n")
f.write("  [./ramp_temperature]\n    initial_condition = 92\n  [../]\n")
f.write("  [./production_temperature]\n    initial_condition = 92 # the production_T Transfer lags one timestep behind for some reason, so give this a reasonable initial condition\n  [../]\n")
for i in range(20):
    f.write("  [./transported_" + var_name[i] + "]\n  [../]\n")
f.write("  [./transported_mass]\n  [../]\n")
for i in range(20):
    f.write("  [./massfrac_" + var_name[i] + "]\n  [../]\n")
for mineral in all_minerals:
    f.write("  [./dumped_" + mineral + "]\n  [../]\n")
f.write("# The production_* Transfers lag one timestep behind for some reason (when the porous_flow simulation has finished, it correctly computes mole_rate_*_produced, but the Transfer gets the mole_rate_*_produced from the previous timestep), so give the production_rate_* reasonable initial conditions, otherwise they will be zero at the start of the simulation.\n")
for i in range(20):
    f.write("  [./production_rate_" + var_name[i] + "]\n")
    # mols/second = mass_fraction * inject_rate * aquifer_height * 1000 / mol_weight
    f.write("    initial_condition = " + str(ic[i] * inject_rate * 10 * 1000 / mol_weight[i]) + "\n")
    f.write("  [../]\n")
f.write("[]\n")

f.write("\n")
f.write("[AuxKernels]\n")
f.write("  [./ramp_temperature]\n    type = FunctionAux\n    variable = ramp_temperature\n    function = 'min(160, max(92, 92 + (160 - 92) * t / 1E5))'\n  [../]\n")
for i in range(20):
    f.write("  [./transported_" + var_name[i] + "_auxk]\n")
    f.write("    type = GeochemistryQuantityAux\n    quantity = transported_moles_in_original_basis\n")
    f.write("    variable = transported_" + var_name[i] + "\n")
    f.write("    species = '" + geochem_vars[i] + "'\n")
    f.write("  [../]\n")
f.write("  [./transported_mass_auxk]\n")
f.write("    type = ParsedAux\n")
f.write("    args = '")
for i in range(20):
    f.write(" transported_" + var_name[i])
f.write("'\n")
f.write("    variable = transported_mass\n")
f.write("    function = '")
for i in range(19):
    f.write(" transported_" + var_name[i] + " * " + str(mol_weight[i]) + " +")
i = 19
f.write(" transported_" + var_name[i] + " * " + str(mol_weight[i]) + "'\n")
f.write("  [../]\n")
for i in range(20):
    f.write("  [./massfrac_" + var_name[i] + "_auxk]\n")
    f.write("    type = ParsedAux\n")
    f.write("    args = 'transported_mass transported_" + var_name[i] + "'\n")
    f.write("    variable = massfrac_" + var_name[i] + "\n")
    f.write("    function = '" + str(mol_weight[i]) + " * transported_" + var_name[i] + " / transported_mass'\n")
    f.write("  [../]\n")
for mineral in all_minerals:
    f.write("  [./dumped_" + mineral + "_auxk]\n")
    f.write("    type = GeochemistryQuantityAux\n")
    f.write("    variable = dumped_" + mineral + "\n")
    f.write("    species = " + mineral + "\n")
    f.write("    quantity = moles_dumped\n  [../]\n")
f.write("[]\n")

f.write("\n")
f.write("[Postprocessors]\n")
for mineral in all_minerals:
    f.write("  [./cumulative_moles_precipitated_" + mineral + "]\n")
    f.write("    type = PointValue\n")
    f.write("    variable = dumped_" + mineral + "\n  [../]\n")
f.write("  [./production_temperature]\n    type = PointValue\n    variable = production_temperature\n  [../]\n  [./mass_heated_this_timestep]\n    type = PointValue\n    variable = transported_mass\n  [../]\n")
f.write("[]\n")

f.write("\n")
f.write("[Outputs]\n  csv = true\n[]\n")

f.write("\n")
write_executioner(f)

f.write("\n")
f.write("[MultiApps]\n  [./porous_flow_sim]\n    type = TransientMultiApp\n    input_files = " + porous_flow_filename + "\n    execute_on = 'timestep_end'\n  [../]\n[]\n")

f.write("\n")
f.write("[Transfers]\n")
f.write("  [./injection_T]\n    type = MultiAppNearestNodeTransfer\n    direction = TO_MULTIAPP\n    multi_app = porous_flow_sim\n    fixed_meshes = true\n    source_variable = 'solution_temperature'\n    variable = 'injection_temperature'\n  [../]\n")
for i in range(20):
    f.write("  [./injection_" + var_name[i] + "]\n")
    f.write("    type = MultiAppNearestNodeTransfer\n    direction = TO_MULTIAPP\n    multi_app = porous_flow_sim\n    fixed_meshes = true\n")
    f.write("    source_variable = 'massfrac_" + var_name[i] + "'\n")
    f.write("    variable = 'injection_rate_massfrac_" + var_name[i] + "'\n")
    f.write("  [../]\n")
f.write("\n")
f.write("  [./production_T]\n    type = MultiAppPostprocessorInterpolationTransfer\n    direction = FROM_MULTIAPP\n    multi_app = porous_flow_sim\n    postprocessor = production_temperature\n    variable = production_temperature\n  [../]\n")
for i in range(20):
    f.write("  [./production_" + var_name[i] + "]\n")
    f.write("    type = MultiAppPostprocessorInterpolationTransfer\n    direction = FROM_MULTIAPP\n    multi_app = porous_flow_sim\n")
    f.write("    postprocessor = mole_rate_" + var_name[i] + "_produced\n")
    f.write("    variable = production_rate_" + var_name[i] + "\n")
    f.write("  [../]\n")
f.write("[]\n")






f.close()
