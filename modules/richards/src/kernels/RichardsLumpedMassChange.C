//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsLumpedMassChange.h"

// MOOSE includes
#include "MooseVariable.h"

// C++ includes
#include <iostream>

registerMooseObject("RichardsApp", RichardsLumpedMassChange);

InputParameters
RichardsLumpedMassChange::validParams()
{
  InputParameters params = TimeKernel::validParams();
  params.addRequiredParam<UserObjectName>(
      "richardsVarNames_UO", "The UserObject that holds the list of Richards variables.");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "density_UO",
      "List of names of user objects that define the fluid density (or densities for "
      "multiphase).  In the multiphase case, for ease of use, the density, Seff and "
      "Sat UserObjects are the same format as for RichardsMaterial, but only the one "
      "relevant for the specific phase is actually used.");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "seff_UO",
      "List of name of user objects that define effective saturation as a function of "
      "porepressure(s)");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "sat_UO",
      "List of names of user objects that define saturation as a function of effective saturation");
  return params;
}

RichardsLumpedMassChange::RichardsLumpedMassChange(const InputParameters & parameters)
  : TimeKernel(parameters),
    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _num_p(_richards_name_UO.num_v()),
    _pvar(_richards_name_UO.richards_var_num(_var.number())),

    _porosity(getMaterialProperty<Real>("porosity")),
    _porosity_old(getMaterialProperty<Real>("porosity_old")),

    // in the following:  first get the userobject names that were inputted, then get the _pvar one
    // of these, then get the actual userobject that this corresponds to, then finally & gives
    // pointer to RichardsDensity (or whatever) object.
    _seff_UO(
        getUserObjectByName<RichardsSeff>(getParam<std::vector<UserObjectName>>("seff_UO")[_pvar])),
    _sat_UO(
        getUserObjectByName<RichardsSat>(getParam<std::vector<UserObjectName>>("sat_UO")[_pvar])),
    _density_UO(getUserObjectByName<RichardsDensity>(
        getParam<std::vector<UserObjectName>>("density_UO")[_pvar]))
{
  _ps_at_nodes.resize(_num_p);
  _ps_old_at_nodes.resize(_num_p);

  for (unsigned int pnum = 0; pnum < _num_p; ++pnum)
  {
    _ps_at_nodes[pnum] = _richards_name_UO.nodal_var(pnum);
    _ps_old_at_nodes[pnum] = _richards_name_UO.nodal_var_old(pnum);
  }

  _dseff.resize(_num_p);
}

Real
RichardsLumpedMassChange::computeQpResidual()
{
  // current values:
  const Real density = _density_UO.density((*_ps_at_nodes[_pvar])[_i]);
  const Real seff = _seff_UO.seff(_ps_at_nodes, _i);
  const Real sat = _sat_UO.sat(seff);
  const Real mass = _porosity[_qp] * density * sat;

  // old values:
  const Real density_old = _density_UO.density((*_ps_old_at_nodes[_pvar])[_i]);
  const Real seff_old = _seff_UO.seff(_ps_old_at_nodes, _i);
  const Real sat_old = _sat_UO.sat(seff_old);
  const Real mass_old = _porosity_old[_qp] * density_old * sat_old;

  return _test[_i][_qp] * (mass - mass_old) / _dt;
}

Real
RichardsLumpedMassChange::computeQpJacobian()
{
  if (_i != _j)
    return 0.0;

  const Real density = _density_UO.density((*_ps_at_nodes[_pvar])[_i]);
  const Real ddensity = _density_UO.ddensity((*_ps_at_nodes[_pvar])[_i]);

  const Real seff = _seff_UO.seff(_ps_at_nodes, _i);
  _seff_UO.dseff(_ps_at_nodes, _i, _dseff);

  const Real sat = _sat_UO.sat(seff);
  const Real dsat = _sat_UO.dsat(seff);

  const Real mass_prime = _porosity[_qp] * (ddensity * sat + density * _dseff[_pvar] * dsat);

  return _test[_i][_qp] * mass_prime / _dt;
}

Real
RichardsLumpedMassChange::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_richards_name_UO.not_richards_var(jvar))
    return 0.0;
  if (_i != _j)
    return 0.0;
  const unsigned int dvar = _richards_name_UO.richards_var_num(jvar);

  const Real density = _density_UO.density((*_ps_at_nodes[_pvar])[_i]);

  const Real seff = _seff_UO.seff(_ps_at_nodes, _i);
  _seff_UO.dseff(_ps_at_nodes, _i, _dseff);

  const Real dsat = _sat_UO.dsat(seff);

  const Real mass_prime = _porosity[_qp] * density * _dseff[dvar] * dsat;

  return _test[_i][_qp] * mass_prime / _dt;
}
