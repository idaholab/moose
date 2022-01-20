//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PackedColumn.h"
#include "Function.h"
#include "DelimitedFileReader.h"

registerMooseObject("DarcyThermoMechApp", PackedColumn);

InputParameters
PackedColumn::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("temperature", "The temperature (C) of the fluid.");

  // Add a parameter to get the radius of the spheres in the column
  // (used later to interpolate permeability).
  params.addParam<FunctionName>("radius",
                                "1.0",
                                "The radius of the steel spheres (mm) that are packed in the "
                                "column for computing permeability.");

  // http://en.wikipedia.org/wiki/Close-packing_of_equal_spheres
  params.addParam<FunctionName>(
      "porosity", 0.25952, "Porosity of porous media, default is for closed packed spheres.");

  // Fluid properties
  params.addParam<Real>(
      "fluid_viscosity", 1.002e-3, "Fluid viscosity (Pa s); default is for water at 20C).");
  params.addParam<FileName>(
      "fluid_viscosity_file",
      "The name of a file containing the fluid viscosity (Pa-s) as a function of temperature "
      "(C); if provided the constant value is ignored.");

  params.addParam<Real>("fluid_thermal_conductivity",
                        0.59803,
                        "Fluid thermal conductivity (W/(mK); default is for water at 20C).");
  params.addParam<FileName>(
      "fluid_thermal_conductivity_file",
      "The name of a file containing fluid thermal conductivity (W/(mK)) as a function of "
      "temperature (C); if provided the constant value is ignored.");

  params.addParam<Real>(
      "fluid_density", 998.21, "Fluid density (kg/m^3); default is for water at 20C).");
  params.addParam<FileName>("fluid_density_file",
                            "The name of a file containing fluid density (kg/m^3) as a function "
                            "of temperature (C); if provided the constant value is ignored.");

  params.addParam<Real>(
      "fluid_specific_heat", 4157.0, "Fluid specific heat (J/(kgK); default is for water at 20C).");
  params.addParam<FileName>(
      "fluid_specific_heat_file",
      "The name of a file containing fluid specific heat (J/(kgK) as a function of temperature "
      "(C); if provided the constant value is ignored.");

  params.addParam<Real>("fluid_thermal_expansion",
                        2.07e-4,
                        "Fluid thermal expansion coefficient (1/K); default is for water at 20C).");
  params.addParam<FileName>("fluid_thermal_expansion_file",
                            "The name of a file containing fluid thermal expansion coefficient "
                            "(1/K) as a function of temperature "
                            "(C); if provided the constant value is ignored.");

  // Solid properties
  // https://en.wikipedia.org/wiki/Stainless_steel#Properties
  params.addParam<Real>("solid_thermal_conductivity",
                        15.0,
                        "Solid thermal conductivity (W/(mK); default is for AISI/ASTIM 304 "
                        "stainless steel at 20C).");
  params.addParam<FileName>(
      "solid_thermal_conductivity_file",
      "The name of a file containing solid thermal conductivity (W/(mK)) as a function of "
      "temperature (C); if provided the constant value is ignored.");

  params.addParam<Real>(
      "solid_density",
      7900,
      "Solid density (kg/m^3); default is for AISI/ASTIM 304 stainless steel at 20C).");
  params.addParam<FileName>("solid_density_file",
                            "The name of a file containing solid density (kg/m^3) as a function "
                            "of temperature (C); if provided the constant value is ignored.");

  params.addParam<Real>(
      "solid_specific_heat",
      500,
      "Solid specific heat (J/(kgK); default is for AISI/ASTIM 304 stainless steel at 20C).");
  params.addParam<FileName>(
      "solid_specific_heat_file",
      "The name of a file containing solid specific heat (J/(kgK) as a function of temperature "
      "(C); if provided the constant value is ignored.");

  params.addParam<Real>("solid_thermal_expansion",
                        17.3e-6,
                        "Solid thermal expansion coefficient (1/K); default is for water at 20C).");
  params.addParam<FileName>("solid_thermal_expansion_file",
                            "The name of a file containing solid thermal expansion coefficient "
                            "(1/K) as a function of temperature "
                            "(C); if provided the constant value is ignored.");
  return params;
}

PackedColumn::PackedColumn(const InputParameters & parameters)
  : Material(parameters),
    // Get the one parameter from the input file
    _input_radius(getFunction("radius")),
    _input_porosity(getFunction("porosity")),
    _temperature(adCoupledValue("temperature")),

    // Fluid
    _fluid_mu(getParam<Real>("fluid_viscosity")),
    _fluid_k(getParam<Real>("fluid_thermal_conductivity")),
    _fluid_rho(getParam<Real>("fluid_density")),
    _fluid_cp(getParam<Real>("fluid_specific_heat")),
    _fluid_cte(getParam<Real>("fluid_thermal_expansion")),

    // Solid
    _solid_k(getParam<Real>("solid_thermal_conductivity")),
    _solid_rho(getParam<Real>("solid_density")),
    _solid_cp(getParam<Real>("solid_specific_heat")),
    _solid_cte(getParam<Real>("solid_thermal_expansion")),

    // Material Properties being produced by this object
    _permeability(declareADProperty<Real>("permeability")),
    _porosity(declareADProperty<Real>("porosity")),
    _viscosity(declareADProperty<Real>("viscosity")),
    _thermal_conductivity(declareADProperty<Real>("thermal_conductivity")),
    _specific_heat(declareADProperty<Real>("specific_heat")),
    _density(declareADProperty<Real>("density")),
    _thermal_expansion(declareADProperty<Real>("thermal_expansion"))
{
  // Set data for permeability interpolation
  std::vector<Real> sphere_sizes = {1, 3};
  std::vector<Real> permeability = {0.8451e-9, 8.968e-9};
  _permeability_interpolation.setData(sphere_sizes, permeability);

  // Fluid viscosity, thermal conductivity, density, and specific heat
  _use_fluid_mu_interp = initInputData("fluid_viscosity_file", _fluid_mu_interpolation);
  _use_fluid_k_interp = initInputData("fluid_thermal_conductivity_file", _fluid_k_interpolation);
  _use_fluid_rho_interp = initInputData("fluid_density_file", _fluid_rho_interpolation);
  _use_fluid_cp_interp = initInputData("fluid_specific_heat_file", _fluid_cp_interpolation);
  _use_fluid_cte_interp = initInputData("fluid_thermal_expansion_file", _fluid_cte_interpolation);

  // Solid thermal conductivity, density, and specific heat
  _use_solid_k_interp = initInputData("solid_thermal_conductivity_file", _solid_k_interpolation);
  _use_solid_rho_interp = initInputData("solid_density_file", _solid_rho_interpolation);
  _use_solid_cp_interp = initInputData("solid_specific_heat_file", _solid_cp_interpolation);
  _use_solid_cte_interp = initInputData("solid_thermal_expansion_file", _solid_cte_interpolation);
}

void
PackedColumn::computeQpProperties()
{
  // Current temperature
  ADReal temp = _temperature[_qp] - 273.15;

  // Permeability
  Real radius_value = _input_radius.value(_t, _q_point[_qp]);
  mooseAssert(radius_value >= 1 && radius_value <= 3,
              "The radius range must be in the range [1, 3], but " << radius_value << " provided.");
  _permeability[_qp] = _permeability_interpolation.sample(radius_value);

  // Porosity
  Real porosity_value = _input_porosity.value(_t, _q_point[_qp]);
  mooseAssert(porosity_value > 0 && porosity_value <= 1,
              "The porosity range must be in the range (0, 1], but " << porosity_value
                                                                     << " provided.");
  _porosity[_qp] = porosity_value;

  // Fluid properties
  _viscosity[_qp] =
      _use_fluid_mu_interp ? _fluid_mu_interpolation.sample(raw_value(temp)) : _fluid_mu;
  ADReal fluid_k = _use_fluid_k_interp ? _fluid_k_interpolation.sample(raw_value(temp)) : _fluid_k;
  ADReal fluid_rho =
      _use_fluid_rho_interp ? _fluid_rho_interpolation.sample(raw_value(temp)) : _fluid_rho;
  ADReal fluid_cp =
      _use_fluid_cp_interp ? _fluid_cp_interpolation.sample(raw_value(temp)) : _fluid_cp;
  ADReal fluid_cte =
      _use_fluid_cte_interp ? _fluid_cte_interpolation.sample(raw_value(temp)) : _fluid_cte;

  // Solid properties
  ADReal solid_k = _use_solid_k_interp ? _solid_k_interpolation.sample(raw_value(temp)) : _solid_k;
  ADReal solid_rho =
      _use_solid_rho_interp ? _solid_rho_interpolation.sample(raw_value(temp)) : _solid_rho;
  ADReal solid_cp =
      _use_solid_cp_interp ? _solid_cp_interpolation.sample(raw_value(temp)) : _solid_cp;
  ADReal solid_cte =
      _use_solid_cte_interp ? _solid_cte_interpolation.sample(raw_value(temp)) : _solid_cte;

  // Compute the heat conduction material properties as a linear combination of
  // the material properties for fluid and steel.
  _thermal_conductivity[_qp] = _porosity[_qp] * fluid_k + (1.0 - _porosity[_qp]) * solid_k;
  _density[_qp] = _porosity[_qp] * fluid_rho + (1.0 - _porosity[_qp]) * solid_rho;
  _specific_heat[_qp] = _porosity[_qp] * fluid_cp + (1.0 - _porosity[_qp]) * solid_cp;
  _thermal_expansion[_qp] = _porosity[_qp] * fluid_cte + (1.0 - _porosity[_qp]) * solid_cte;
}

bool
PackedColumn::initInputData(const std::string & param_name, ADLinearInterpolation & interp)
{
  if (isParamValid(param_name))
  {
    const std::string & filename = getParam<FileName>(param_name);
    MooseUtils::DelimitedFileReader reader(filename, &_communicator);
    reader.setComment("#");
    reader.read();
    interp.setData(reader.getData(0), reader.getData(1));
    return true;
  }
  return false;
}
