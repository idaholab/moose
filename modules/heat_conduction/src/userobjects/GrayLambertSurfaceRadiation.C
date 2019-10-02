//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrayLambertSurfaceRadiation.h"
#include "MathUtils.h"
#include "Function.h"
#include "libmesh/quadrature.h"

#include <cmath>

registerMooseObject("HeatConductionApp", GrayLambertSurfaceRadiation);

template <>
InputParameters
validParams<GrayLambertSurfaceRadiation>()
{
  InputParameters params = validParams<SideUserObject>();
  params.addParam<Real>(
      "stefan_boltzmann_constant",
      5.670367e-8,
      "The Stefan-Boltzmann constant. Default value is in units of [W / m^2 K^4].");
  params.addRequiredCoupledVar("temperature", "The coupled temperature variable.");
  params.addRequiredParam<std::vector<Real>>("emissivity", "Emissivities for each boundary.");
  params.addRequiredParam<std::vector<std::vector<Real>>>(
      "view_factors", "The view factors from sideset i to sideset j.");
  params.addParam<std::vector<BoundaryName>>(
      "fixed_temperature_boundary",
      "The list of boundary IDs from the mesh with fixed temperatures.");
  params.addParam<std::vector<FunctionName>>("fixed_boundary_temperatures",
                                             "The temperatures of the fixed boundary.");
  params.addParam<std::vector<BoundaryName>>(
      "adiabatic_boundary", "The list of boundary IDs from the mesh that are adiabatic.");

  params.addClassDescription(
      "This object implements the exchange of heat by radiation between sidesets.");
  return params;
}

GrayLambertSurfaceRadiation::GrayLambertSurfaceRadiation(const InputParameters & parameters)
  : SideUserObject(parameters),
    _sigma_stefan_boltzmann(getParam<Real>("stefan_boltzmann_constant")),
    _n_sides(boundaryIDs().size()),
    _temperature(coupledValue("temperature")),
    _emissivity(getParam<std::vector<Real>>("emissivity")),
    _view_factors(getParam<std::vector<std::vector<Real>>>("view_factors")),
    _radiosity(_n_sides),
    _heat_flux_density(_n_sides),
    _side_temperature(_n_sides),
    _side_type(_n_sides),
    _areas(_n_sides),
    _beta(_n_sides)
{
  // set up the map from the side id to the local index & check
  // note that boundaryIDs is not in the right order anymore!
  {
    std::vector<BoundaryName> boundary_names = getParam<std::vector<BoundaryName>>("boundary");

    for (unsigned int j = 0; j < boundary_names.size(); ++j)
    {
      if (boundary_names[j] == "ANY_BOUNDARY_ID")
        paramError("boundary", "boundary must be explicitly provided.");

      _side_id_index[_mesh.getBoundaryID(boundary_names[j])] = j;
      _side_type[j] = VARIABLE_TEMPERATURE;
    }

    // consistency check on emissivity, must be as many entries as boundary
    if (boundary_names.size() != _emissivity.size())
      paramError("emissivity", "The number of entries must match the number of boundary entries.");
  }

  /*
   * View factors must be provided as square array, while this seems redundant, it is necessary
   * for enforcing energy continuity, i.e. sum of a row of the view factor matrix must be 1
   * If this condition does not hold, a coupled heat equation, radiative transport problem
   * will not converge.
   */
  if (_view_factors.size() != _n_sides)
    paramError("view_factors",
               "Leading dimension of view_factors must be equal to number of side sets.");

  for (unsigned int i = 0; i < _n_sides; ++i)
  {
    if (_view_factors[i].size() != _n_sides)
      paramError("view_factors",
                 "view_factors must be provided as square array. Row ",
                 i,
                 " has ",
                 _view_factors[i].size(),
                 " entries.");

    // check row-sum and normalize if necessary
    Real sum = 0;
    for (auto & v : _view_factors[i])
      sum += v;

    // an error of 5% is acceptable, but more indicates an error in the
    // problem setup
    if (std::abs(sum - 1) > 0.05)
      paramError("view_factors", "view_factors row ", i, " sums to ", sum);

    // correct view factors
    for (auto & v : _view_factors[i])
      v /= sum;
  }

  // get the fixed boundaries of the system if any are provided
  if (isParamValid("fixed_temperature_boundary"))
  {
    // if fixed_temperature_boundary is valid, then fixed_side_temperatures must be
    // valid, too
    if (!isParamValid("fixed_boundary_temperatures"))
      paramError("fixed_boundary_temperatures",
                 "fixed_temperature_boundary is provided, but fixed_boundary_temperatures is not.");

    auto fst_fn = getParam<std::vector<FunctionName>>("fixed_boundary_temperatures");
    for (auto & fn : fst_fn)
      _fixed_side_temperature.push_back(&getFunctionByName(fn));

    // get the fixed boundaries and temperatures
    std::vector<BoundaryName> boundary_names =
        getParam<std::vector<BoundaryName>>("fixed_temperature_boundary");

    if (boundary_names.size() != _fixed_side_temperature.size())
      paramError(
          "fixed_boundary_temperatures",
          "fixed_boundary_temperatures and fixed_temperature_boundary must have the same length.");

    unsigned int index = 0;
    for (auto & name : boundary_names)
    {
      _fixed_side_id_index[_mesh.getBoundaryID(name)] = index;
      index++;
    }

    // check that fixed side ids is a subset of boundary ids
    // and update _side_type info
    for (auto & p : _fixed_side_id_index)
    {
      if (_side_id_index.find(p.first) == _side_id_index.end())
        paramError("fixed_temperature_boundary",
                   "fixed_temperature_boundary must be a subset of boundary.");
      _side_type[_side_id_index.find(p.first)->second] = FIXED_TEMPERATURE;
    }
  }

  // get the fixed boundaries of the system if any are provided
  if (isParamValid("adiabatic_boundary"))
  {
    // get the adiabatic boundaries and temperatures
    std::vector<BoundaryName> boundary_names =
        getParam<std::vector<BoundaryName>>("adiabatic_boundary");

    for (auto & name : boundary_names)
      _adiabatic_side_ids.insert(_mesh.getBoundaryID(name));

    // check that adiabatic side ids is a subset of boundary ids
    // and update _side_type info
    for (auto & id : _adiabatic_side_ids)
    {
      if (_side_id_index.find(id) == _side_id_index.end())
        paramError("adiabatic_boundary", "adiabatic_boundary must be a subset of boundary.");
      _side_type[_side_id_index.find(id)->second] = ADIABATIC;
    }

    // make sure that adiabatic boundaries are not already fixed boundaries
    for (auto & id : _adiabatic_side_ids)
      if (_fixed_side_id_index.find(id) != _fixed_side_id_index.end())
        paramError("adiabatic_boundary", "Isothermal boundary cannot also be adiabatic boundary.");
  }
}

void
GrayLambertSurfaceRadiation::execute()
{
  mooseAssert(_side_id_index.find(_current_boundary_id) != _side_id_index.end(),
              "Current boundary id not in _side_id_index.");
  unsigned int index = _side_id_index.find(_current_boundary_id)->second;

  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    _areas[index] += _JxW[qp] * _coord[qp];

    Real temp = 0;
    if (_side_type[index] == ADIABATIC)
      continue;
    else if (_side_type[index] == VARIABLE_TEMPERATURE)
      temp = _temperature[qp];
    else if (_side_type[index] == FIXED_TEMPERATURE)
    {
      unsigned int iso_index = _fixed_side_id_index.find(_current_boundary_id)->second;
      temp = _fixed_side_temperature[iso_index]->value(_t, _q_point[qp]);
    }

    _beta[index] += _JxW[qp] * _coord[qp] * _sigma_stefan_boltzmann * _emissivity[index] *
                    MathUtils::pow(temp, 4);
    _side_temperature[index] += _JxW[qp] * _coord[qp] * temp;
  }
}

void
GrayLambertSurfaceRadiation::initialize()
{
  for (unsigned int j = 0; j < _n_sides; ++j)
  {
    _areas[j] = 0;
    _beta[j] = 0;
    _side_temperature[j] = 0;
  }
}

void
GrayLambertSurfaceRadiation::finalize()
{
  // need to do some parallel communiction here
  gatherSum(_areas);
  gatherSum(_beta);
  gatherSum(_side_temperature);

  // first compute averages from the totals
  for (unsigned int j = 0; j < _n_sides; ++j)
  {
    _beta[j] /= _areas[j];
    _side_temperature[j] /= _areas[j];
  }

  // matrix and rhs vector for the view factor calculation
  DenseMatrix<Real> matrix(_n_sides, _n_sides);
  DenseVector<Real> rhs(_n_sides);
  DenseVector<Real> radiosity(_n_sides);
  for (unsigned int i = 0; i < _n_sides; ++i)
  {
    rhs(i) = _beta[i];
    matrix(i, i) = 1;
    for (unsigned int j = 0; j < _n_sides; ++j)
    {
      if (_side_type[i] == ADIABATIC)
        matrix(i, j) -= _view_factors[i][j];
      else
        matrix(i, j) -= (1 - _emissivity[i]) * _view_factors[i][j];
    }
  }

  // compute the radiosityes
  matrix.lu_solve(rhs, radiosity);

  // store the radiosity, temperatures and heat flux density for each surface
  for (unsigned int i = 0; i < _n_sides; ++i)
  {
    _radiosity[i] = radiosity(i);

    // _heat_flux_density is obtained from a somewhat cumbersome relation
    // but it has the advantage that we do not divide by 1 - emissivity
    // which blows up for black bodies
    _heat_flux_density[i] = radiosity(i);
    for (unsigned int j = 0; j < _n_sides; ++j)
      _heat_flux_density[i] -= _view_factors[i][j] * radiosity(j);

    if (_side_type[i] == ADIABATIC)
      _side_temperature[i] =
          std::pow((radiosity(i) + (1 - _emissivity[i]) / _emissivity[i] * _heat_flux_density[i]) /
                       _sigma_stefan_boltzmann,
                   0.25);
  }
}

void
GrayLambertSurfaceRadiation::threadJoin(const UserObject & y)
{
  const GrayLambertSurfaceRadiation & pps = static_cast<const GrayLambertSurfaceRadiation &>(y);

  for (unsigned int j = 0; j < _n_sides; ++j)
  {
    _areas[j] += pps._areas[j];
    _side_temperature[j] += pps._side_temperature[j];
    _beta[j] += pps._beta[j];
  }
}

Real
GrayLambertSurfaceRadiation::getSurfaceHeatFluxDensity(BoundaryID id) const
{
  if (_side_id_index.find(id) == _side_id_index.end())
    return 0;
  return _heat_flux_density[_side_id_index.find(id)->second];
}

Real
GrayLambertSurfaceRadiation::getSurfaceTemperature(BoundaryID id) const
{
  if (_side_id_index.find(id) == _side_id_index.end())
    return 0;
  return _side_temperature[_side_id_index.find(id)->second];
}

Real
GrayLambertSurfaceRadiation::getSurfaceRadiosity(BoundaryID id) const
{
  if (_side_id_index.find(id) == _side_id_index.end())
    return 0;
  return _radiosity[_side_id_index.find(id)->second];
}

Real
GrayLambertSurfaceRadiation::getSurfaceEmissivity(BoundaryID id) const
{
  if (_side_id_index.find(id) == _side_id_index.end())
    return 1;
  return _emissivity[_side_id_index.find(id)->second];
}
