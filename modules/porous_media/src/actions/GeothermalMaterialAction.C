#include "GeothermalMaterialAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<GeothermalMaterialAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<SubdomainName> >("block", "The list of ids of the blocks (subdomain) that these materials will be applied to");
//H, T, M, C, TH, THM, TM, HM, THMC ...
  params.addRequiredParam<bool>("solid_mechanics", "Solid mechancis material");
  params.addRequiredParam<bool>("heat_transport", "Heat transport material");
  params.addRequiredParam<bool>("fluid_flow", "Fluid flow material");
  params.addRequiredParam<bool>("chemical_reactions", "Chemcial reactions material");
//Main non-linear coupled variables
  params.addParam<NonlinearVariableName>("pressure", "Coupled pressure variable, [Pa]");
  params.addParam<NonlinearVariableName>("temperature", "Coupled temperature variable, [K]");
  params.addParam<NonlinearVariableName>("enthalpy", "Coupled enthalpy variable, [J]");
  params.addParam<NonlinearVariableName>("x_disp", "Coupled x_displacement variable, [m]");
  params.addParam<NonlinearVariableName>("y_disp", "Coupled y_displacement variable, [m]");
  params.addParam<NonlinearVariableName>("z_disp", "Coupled z_displacement variable, [m]");
  params.addParam<std::vector<NonlinearVariableName> >("v", "The list of primary species to add");
//Input parameters
  //porous_media
  params.addParam<Real>("permeability", 1e-12, "[m^2]");
  params.addParam<Real>("porosity", 0.3,"dimentionless");
  params.addParam<Real>("compressibility", 1e-15,"dimentionless");
  params.addParam<Real>("density_rock", 2.5e3, "[kg/m^3]");
  params.addParam<Real>("gravity", 9.80665, "[m/s^2]");
  params.addParam<Real>("gx", 0.0, "dimentionless");
  params.addParam<Real>("gy", 0.0, "dimentionless");
  params.addParam<Real>("gz", 1.0, "dimentionless");
  //solid_mechanics
  params.addParam<Real>("biot_coeff", 1.0, "dimentionless");
  params.addParam<Real>("biot_modulus", 2.5e10, "dimenstionless");
  params.addParam<Real>("poissons_ratio", 0.2, "dimentionless");
  params.addParam<Real>("thermal_expansion", 1e-6, "[1/K]");
  params.addParam<Real>("thermal_strain_ref_temp", 293.15, "[K]");
  params.addParam<Real>("youngs_modulus", 1.5e10, "[Pa]");
  //heat_transport
  params.addParam<Real>("specific_heat_rock", 0.92e3, "[J/kg.K]");
  params.addParam<Real>("specific_heat_water", 4.186e3, "[J/kg.K]");
  params.addParam<Real>("thermal_conductivity", 2.5, "[W/m.K]");
  //fluid_flow
  params.addParam<Real>("constant_density", 1000, "[kg/m^3]");
  params.addParam<Real>("constant_viscosity", 0.12e-3, "[Pa.s]");
  params.addParam<bool>("temp_dependent_fluid_props", true, "flag true if single-phase and fluid properties are temperature dependent");
  params.addParam<UserObjectName>("water_steam_properties", "If temp_dependent_fluid_props = true, select which user object to use for EOS calculations");
  //chemical_reactions
  params.addParam<Real>("diffusivity", 1e-8, "[kg/m^3]");
  params.addParam<std::vector<Real> >("mineral", std::vector<Real>(1, 16.65), "[mol/L] solution");
  params.addParam<std::vector<Real> >("mineral_density", std::vector<Real>(1, 100.08), "[g/cm^3]");
  params.addParam<std::vector<Real> >("molecular_weight", std::vector<Real>(1, 2.5), "[g/mol]");

  return params;
}

GeothermalMaterialAction::GeothermalMaterialAction(const std::string & name, InputParameters params) :
  Action(name, params),
//H, T, M, C, TH, THM, TM, HM, THMC ...
    _has_heat_tran(getParam<bool>("heat_transport")),       //T
    _has_fluid_flow(getParam<bool>("fluid_flow")),          //H
    _has_solid_mech(getParam<bool>("solid_mechanics")),     //M
    _has_chem_react(getParam<bool>("chemical_reactions"))   //C
{
}

void
GeothermalMaterialAction::act()
{
    // input parameters for this action are split up into 5 sets of parameters
    // shared_params = all paramerters of the base class and all valid NL variables,
    //                 this gets added to each of the following 4 parameter sets
    // sm_params = all parameters needed for SolidMechanics material
    // ht_params = all parameters needed for HeatTransport material
    // ff_params = all parameters needed for FluidFlow material
    // cr_params = all parameters needed for ChemicalReactions material

    //get input parameters from base class PorousMedia
    InputParameters shared_params = _factory.getValidParams("PorousMedia");
    //get block #/name that we want to assign this material action to and add it to the shared_params
    std::vector<SubdomainName> block = getParam<std::vector<SubdomainName> >("block");
    shared_params.set<std::vector<SubdomainName> >("block") = block;

    //check which NL variables are coupled and add them to shared_params if valid
    if (_pars.isParamValid("x_disp"))
    {
        std::vector<NonlinearVariableName> x_var (1, getParam<NonlinearVariableName>("x_disp"));
        shared_params.set<std::vector<NonlinearVariableName> >("x_disp") = x_var;
    }
    if (_pars.isParamValid("y_disp"))
    {
        std::vector<NonlinearVariableName> y_var (1, getParam<NonlinearVariableName>("y_disp"));
        shared_params.set<std::vector<NonlinearVariableName> >("y_disp") = y_var;
    }
    if (_pars.isParamValid("z_disp"))
    {
        std::vector<NonlinearVariableName> z_var (1, getParam<NonlinearVariableName>("z_disp"));
        shared_params.set<std::vector<NonlinearVariableName> >("z_disp") = z_var;
    }
    if (_pars.isParamValid("pressure"))
    {
        std::vector<NonlinearVariableName> press_var (1, getParam<NonlinearVariableName>("pressure"));
        shared_params.set<std::vector<NonlinearVariableName> >("pressure") = press_var;
    }
    if (_pars.isParamValid("enthalpy"))
    {
        std::vector<NonlinearVariableName> enth_var (1, getParam<NonlinearVariableName>("enthalpy"));
        shared_params.set<std::vector<NonlinearVariableName> >("enthalpy") = enth_var;
    }
    if (_pars.isParamValid("temperature"))
    {
        std::vector<NonlinearVariableName> temp_var (1, getParam<NonlinearVariableName>("temperature"));
        shared_params.set<std::vector<NonlinearVariableName> >("temperature") = temp_var;
    }
    if (_pars.isParamValid("v"))
    {
        std::vector<NonlinearVariableName> chem_vars = getParam<std::vector<NonlinearVariableName> >("v");
        shared_params.set<std::vector<NonlinearVariableName> >("v") = chem_vars;
    }

    //get base class (PorousMedia) paramerters from input
    Real permeability = getParam<Real>("permeability");
    Real porosity = getParam<Real>("porosity");
    Real compressibility = getParam<Real>("compressibility");
    Real density_rock = getParam<Real>("density_rock");
    Real gravity = getParam<Real>("gravity");
    Real gx = getParam<Real>("gx");
    Real gy = getParam<Real>("gy");
    Real gz = getParam<Real>("gz");

    //add these base class paramerters to shared_params, since all dependent classes need these parameters
    shared_params.set<Real>("permeability") = permeability;
    shared_params.set<Real>("porosity") = porosity;
    shared_params.set<Real>("compressibility") = compressibility;

    shared_params.set<Real>("density_rock") = density_rock;
    shared_params.set<Real>("gravity") = gravity;
    shared_params.set<Real>("gx") = gx;
    shared_params.set<Real>("gy") = gy;
    shared_params.set<Real>("gz") = gz;
    shared_params.set<bool>("has_chem_reactions") = _has_chem_react;

    // based upon user input for this action, we will appropriately add materials (and their respective parameters)
    // for a customizable mix-and-match THMC material

    // hydro problems: _has_fluid_flow = true
    // thermo problems: _has_heat_tran = true
    // mechanical problems: _has_solid_mech = true
    // chemical problems: _has_chem_react = true

    if (_has_fluid_flow)
        addFluidFlowMaterial(shared_params);

    if (_has_heat_tran)
        addHeatTransportMaterial(shared_params);

    if (_has_solid_mech)
        addSolidMechanicsMaterial(shared_params);

    if (_has_chem_react)
        addChemicalReactionsMaterial(shared_params);
}

void
GeothermalMaterialAction::addSolidMechanicsMaterial(InputParameters shared_params)
{
    //get input parameters from SolidMechanics material
    InputParameters sm_params = _factory.getValidParams("SolidMechanics");

    //add shared_params from above
    sm_params += shared_params;

    //get SolidMechancis paramerters from input file
    Real poissons_ratio = getParam<Real>("poissons_ratio");
    Real biot_coeff = getParam<Real>("biot_coeff");
    Real biot_modulus = getParam<Real>("biot_modulus");
    Real thermal_expansion = getParam<Real>("thermal_expansion");
    Real youngs_modulus = getParam<Real>("youngs_modulus");
    Real thermal_strain_ref_temp = getParam<Real>("thermal_strain_ref_temp");

    //add these paramerters to sm_params
    sm_params.set<Real>("poissons_ratio") = poissons_ratio;
    sm_params.set<Real>("biot_coeff") = biot_coeff;
    sm_params.set<Real>("biot_modulus") = biot_modulus;
    sm_params.set<Real>("thermal_expansion") = thermal_expansion;
    sm_params.set<Real>("youngs_modulus") = youngs_modulus;
    sm_params.set<Real>("thermal_strain_ref_temp") = thermal_strain_ref_temp;

    //add SolidMechanics material using sm_params
    _problem->addMaterial("SolidMechanics", "solid_mechanics", sm_params);
}

void
GeothermalMaterialAction::addHeatTransportMaterial(InputParameters shared_params)
{
    //get input parameters from HeatTransport material
    InputParameters ht_params = _factory.getValidParams("HeatTransport");

    //add shared_params from above
    ht_params += shared_params;

    //get HeatTransport paramerters from input file
    Real specific_heat_rock = getParam<Real>("specific_heat_rock");
    Real specific_heat_water = getParam<Real>("specific_heat_water");
    Real thermal_conductivity = getParam<Real>("thermal_conductivity");

    //add these paramerters to ht_params
    ht_params.set<Real>("specific_heat_rock") = specific_heat_rock;
    ht_params.set<Real>("specific_heat_water") = specific_heat_water;
    ht_params.set<Real>("thermal_conductivity") = thermal_conductivity;

    //add HeatTransport material using ht_params
    _problem->addMaterial("HeatTransport", "heat_transport", ht_params);
}

void
GeothermalMaterialAction::addFluidFlowMaterial(InputParameters shared_params)
{
    //get input parameters from FliudFlow material
    InputParameters ff_params = _factory.getValidParams("FluidFlow");

    //add shared_params from above
    ff_params += shared_params;

    //get FluidFlow paramerters from input file and add these paramerters to ff_params
    bool temp_dependent_fluid_props = getParam<bool>("temp_dependent_fluid_props");
    ff_params.set<bool>("temp_dependent_fluid_props") = temp_dependent_fluid_props;

    //if temp_dependent_fluid_props = true, then we need to also grab the UserObject that
    //calculates our water/steam EOS
    if (temp_dependent_fluid_props == true)
    {
        UserObjectName water_steam_properties = getParam<UserObjectName>("water_steam_properties");
        ff_params.set<UserObjectName>("water_steam_properties") = water_steam_properties;
    }
    //else we just grab the constant fluid density and viscosity values
    else
    {
        Real constant_density = getParam<Real>("constant_density");
        Real constant_viscosity = getParam<Real>("constant_viscosity");
        ff_params.set<Real>("constant_density") = constant_density;
        ff_params.set<Real>("constant_viscosity") = constant_viscosity;
    }

    //add FluidFlow material using ff_params
    _problem->addMaterial("FluidFlow", "fluid_flow", ff_params);
}

void
GeothermalMaterialAction::addChemicalReactionsMaterial(InputParameters shared_params)
{
    //get input parameters from ChemicalReactions material
    InputParameters cr_params = _factory.getValidParams("ChemicalReactions");

    //add shared_params from above
    cr_params += shared_params;

    //get ChemicalReactions paramerters from input file
    bool has_chem_reactions = true;
    Real diffusivity = getParam<Real>("diffusivity");
    std::vector<Real> mineral = getParam<std::vector<Real> >("mineral");
    std::vector<Real> molecular_weight = getParam<std::vector<Real> >("molecular_weight");
    std::vector<Real> mineral_density = getParam<std::vector<Real> >("mineral_density");

    //add these paramerters to cr_params
    cr_params.set<bool>("has_chem_reactions") = has_chem_reactions;
    cr_params.set<Real>("diffusivity") = diffusivity;
    cr_params.set<std::vector<Real> >("mineral") = mineral;
    cr_params.set<std::vector<Real> >("molecular_weight") = molecular_weight;
    cr_params.set<std::vector<Real> >("mineral_density") = mineral_density;

    //add ChemicalReactions material using cr_params
    _problem->addMaterial("ChemicalReactions", "chemical_reactions", cr_params);
}
