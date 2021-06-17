//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsPiecewiseLinearSink.h"

// MOOSE includes
#include "MooseVariable.h"

// C++ includes
#include <iostream>

registerMooseObject("RichardsApp", RichardsPiecewiseLinearSink);

InputParameters
RichardsPiecewiseLinearSink::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<bool>(
      "use_mobility",
      "If true, then fluxes are multiplied by (density*permeability_nn/viscosity), "
      "where the '_nn' indicates the component normal to the boundary.  In this "
      "case bare_flux is measured in Pa.s^-1.  This can be used in conjunction "
      "with use_relperm.");
  params.addRequiredParam<bool>("use_relperm",
                                "If true, then fluxes are multiplied by relative "
                                "permeability.  This can be used in conjunction "
                                "with use_mobility");
  params.addParam<std::vector<UserObjectName>>("relperm_UO",
                                               "List of names of user objects that "
                                               "define relative permeability.  Only "
                                               "needed if fully_upwind is used");
  params.addParam<std::vector<UserObjectName>>(
      "seff_UO",
      "List of name of user objects that define effective saturation as a function of "
      "pressure list.  Only needed if fully_upwind is used");
  params.addParam<std::vector<UserObjectName>>("density_UO",
                                               "List of names of user objects that "
                                               "define the fluid density.  Only "
                                               "needed if fully_upwind is used");
  params.addRequiredParam<std::vector<Real>>(
      "pressures", "Tuple of pressure values.  Must be monotonically increasing.");
  params.addRequiredParam<std::vector<Real>>(
      "bare_fluxes",
      "Tuple of flux values (measured in kg.m^-2.s^-1 for use_mobility=false, and "
      "in Pa.s^-1 if use_mobility=true).  This flux is OUT of the medium: hence "
      "positive values of flux means this will be a SINK, while negative values "
      "indicate this flux will be a SOURCE.  A piecewise-linear fit is performed to "
      "the (pressure,bare_fluxes) pairs to obtain the flux at any arbitrary "
      "pressure, and the first or last bare_flux values are used if the quad-point "
      "pressure falls outside this range.");
  params.addParam<FunctionName>("multiplying_fcn",
                                1.0,
                                "If this function is provided, the flux "
                                "will be multiplied by this function.  "
                                "This is useful for spatially or "
                                "temporally varying sinks");
  params.addRequiredParam<UserObjectName>(
      "richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  params.addParam<bool>("fully_upwind", false, "Use full upwinding");
  params.addParam<PostprocessorName>(
      "area_pp",
      1,
      "An area postprocessor.  If given, the bare_fluxes will be divided by this "
      "quantity.  This means the bare fluxes are measured in kg.s^-1.  This is "
      "useful for the case when you wish to provide the *total* flux, and let MOOSE "
      "proportion it uniformly across the boundary.  In that case you would have "
      "use_mobility=false=use_relperm, and only one bare flux should be specified");
  return params;
}

RichardsPiecewiseLinearSink::RichardsPiecewiseLinearSink(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _use_mobility(getParam<bool>("use_mobility")),
    _use_relperm(getParam<bool>("use_relperm")),
    _fully_upwind(getParam<bool>("fully_upwind")),

    _sink_func(getParam<std::vector<Real>>("pressures"),
               getParam<std::vector<Real>>("bare_fluxes")),

    _m_func(getFunction("multiplying_fcn")),

    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _num_p(_richards_name_UO.num_v()),
    _pvar(_richards_name_UO.richards_var_num(_var.number())),

    // in the following, getUserObjectByName returns a reference (an alias) to a RichardsBLAH user
    // object, and the & turns it into a pointer
    _density_UO(_fully_upwind ? &getUserObjectByName<RichardsDensity>(
                                    getParam<std::vector<UserObjectName>>("density_UO")[_pvar])
                              : NULL),
    _seff_UO(_fully_upwind ? &getUserObjectByName<RichardsSeff>(
                                 getParam<std::vector<UserObjectName>>("seff_UO")[_pvar])
                           : NULL),
    _relperm_UO(_fully_upwind ? &getUserObjectByName<RichardsRelPerm>(
                                    getParam<std::vector<UserObjectName>>("relperm_UO")[_pvar])
                              : NULL),

    _area_pp(getPostprocessorValue("area_pp")),

    _num_nodes(0),
    _nodal_density(0),
    _dnodal_density_dv(0),
    _nodal_relperm(0),
    _dnodal_relperm_dv(0),

    _pp(getMaterialProperty<std::vector<Real>>("porepressure")),
    _dpp_dv(getMaterialProperty<std::vector<std::vector<Real>>>("dporepressure_dv")),

    _viscosity(getMaterialProperty<std::vector<Real>>("viscosity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),

    _dseff_dv(getMaterialProperty<std::vector<std::vector<Real>>>("ds_eff_dv")),

    _rel_perm(getMaterialProperty<std::vector<Real>>("rel_perm")),
    _drel_perm_dv(getMaterialProperty<std::vector<std::vector<Real>>>("drel_perm_dv")),

    _density(getMaterialProperty<std::vector<Real>>("density")),
    _ddensity_dv(getMaterialProperty<std::vector<std::vector<Real>>>("ddensity_dv"))
{
  _ps_at_nodes.resize(_num_p);
  for (unsigned int pnum = 0; pnum < _num_p; ++pnum)
    _ps_at_nodes[pnum] = _richards_name_UO.nodal_var(pnum);
}

void
RichardsPiecewiseLinearSink::prepareNodalValues()
{
  // NOTE: i'm assuming that all the richards variables are pressure values

  _num_nodes = (*_ps_at_nodes[_pvar]).size();

  Real p;
  Real seff;
  std::vector<Real> dseff_dp;
  Real drelperm_ds;

  _nodal_density.resize(_num_nodes);
  _dnodal_density_dv.resize(_num_nodes);
  _nodal_relperm.resize(_num_nodes);
  _dnodal_relperm_dv.resize(_num_nodes);
  dseff_dp.resize(_num_p);
  for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
  {
    // retrieve and calculate basic things at the node
    p = (*_ps_at_nodes[_pvar])[nodenum]; // pressure of fluid _pvar at node nodenum

    _nodal_density[nodenum] = _density_UO->density(p); // density of fluid _pvar at node nodenum
    _dnodal_density_dv[nodenum].resize(_num_p);
    for (unsigned int ph = 0; ph < _num_p; ++ph)
      _dnodal_density_dv[nodenum][ph] = 0;
    _dnodal_density_dv[nodenum][_pvar] = _density_UO->ddensity(p); // d(density)/dP

    seff = _seff_UO->seff(_ps_at_nodes,
                          nodenum); // effective saturation of fluid _pvar at node nodenum
    _seff_UO->dseff(
        _ps_at_nodes, nodenum, dseff_dp); // d(seff)/d(P_ph), for ph = 0, ..., _num_p - 1

    _nodal_relperm[nodenum] =
        _relperm_UO->relperm(seff); // relative permeability of fluid _pvar at node nodenum
    drelperm_ds = _relperm_UO->drelperm(seff); // d(relperm)/dseff

    _dnodal_relperm_dv[nodenum].resize(_num_p);
    for (unsigned int ph = 0; ph < _num_p; ++ph)
      _dnodal_relperm_dv[nodenum][ph] = drelperm_ds * dseff_dp[ph];
  }
}

void
RichardsPiecewiseLinearSink::computeResidual()
{
  if (_fully_upwind)
    prepareNodalValues();
  IntegratedBC::computeResidual();
}

Real
RichardsPiecewiseLinearSink::computeQpResidual()
{
  Real flux = 0;
  Real k = 0;

  if (!_fully_upwind)
  {
    flux = _test[_i][_qp] * _sink_func.sample(_pp[_qp][_pvar]);
    if (_use_mobility)
    {
      k = (_permeability[_qp] * _normals[_qp]) * _normals[_qp];
      flux *= _density[_qp][_pvar] * k / _viscosity[_qp][_pvar];
    }
    if (_use_relperm)
      flux *= _rel_perm[_qp][_pvar];
  }
  else
  {
    flux = _test[_i][_qp] * _sink_func.sample((*_ps_at_nodes[_pvar])[_i]);
    if (_use_mobility)
    {
      k = (_permeability[0] * _normals[_qp]) * _normals[_qp]; // assume that _permeability is
                                                              // constant throughout element so
                                                              // doesn't need to be upwinded
      flux *= _nodal_density[_i] * k /
              _viscosity[0][_pvar]; // assume that viscosity is constant throughout element
    }
    if (_use_relperm)
      flux *= _nodal_relperm[_i];
  }

  flux *= _m_func.value(_t, _q_point[_qp]);

  if (_area_pp == 0.0)
  {
    if (flux != 0)
      mooseError("RichardsPiecewiseLinearSink: flux is nonzero, but area is zero!\n");
    // if flux == 0, then leave it as zero.
  }
  else
    flux /= _area_pp;

  return flux;
}

void
RichardsPiecewiseLinearSink::computeJacobian()
{
  if (_fully_upwind)
    prepareNodalValues();
  IntegratedBC::computeJacobian();
}

Real
RichardsPiecewiseLinearSink::computeQpJacobian()
{
  return jac(_pvar);
}

void
RichardsPiecewiseLinearSink::computeOffDiagJacobian(const unsigned int jvar)
{
  if (_fully_upwind)
    prepareNodalValues();
  IntegratedBC::computeOffDiagJacobian(jvar);
}

Real
RichardsPiecewiseLinearSink::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_richards_name_UO.not_richards_var(jvar))
    return 0.0;
  unsigned int dvar = _richards_name_UO.richards_var_num(jvar);
  return jac(dvar);
}

Real
RichardsPiecewiseLinearSink::jac(unsigned int wrt_num)
{
  Real flux = 0;
  Real deriv = 0;
  Real k = 0;
  Real mob = 0;
  Real mobp = 0;
  Real phi = 0;

  if (!_fully_upwind)
  {
    flux = _sink_func.sample(_pp[_qp][_pvar]);
    deriv = _sink_func.sampleDerivative(_pp[_qp][_pvar]) * _dpp_dv[_qp][_pvar][wrt_num];
    phi = _phi[_j][_qp];
    if (_use_mobility)
    {
      k = (_permeability[_qp] * _normals[_qp]) * _normals[_qp];
      mob = _density[_qp][_pvar] * k / _viscosity[_qp][_pvar];
      mobp = _ddensity_dv[_qp][_pvar][wrt_num] * k / _viscosity[_qp][_pvar];
      deriv = mob * deriv + mobp * flux;
      flux *= mob;
    }
    if (_use_relperm)
      deriv = _rel_perm[_qp][_pvar] * deriv + _drel_perm_dv[_qp][_pvar][wrt_num] * flux;
  }
  else
  {
    if (_i != _j)
      return 0.0; // residual at node _i only depends on variables at that node
    flux = _sink_func.sample((*_ps_at_nodes[_pvar])[_i]);
    deriv = (_pvar == wrt_num ? _sink_func.sampleDerivative((*_ps_at_nodes[_pvar])[_i])
                              : 0); // NOTE: i'm assuming that the variables are pressure variables
    phi = 1;
    if (_use_mobility)
    {
      k = (_permeability[0] * _normals[_qp]) * _normals[_qp];
      mob = _nodal_density[_i] * k / _viscosity[0][_pvar];
      mobp = _dnodal_density_dv[_i][wrt_num] * k / _viscosity[0][_pvar];
      deriv = mob * deriv + mobp * flux;
      flux *= mob;
    }
    if (_use_relperm)
      deriv = _nodal_relperm[_i] * deriv + _dnodal_relperm_dv[_i][wrt_num] * flux;
  }

  deriv *= _m_func.value(_t, _q_point[_qp]);

  if (_area_pp == 0.0)
  {
    if (deriv != 0)
      mooseError("RichardsPiecewiseLinearSink: deriv is nonzero, but area is zero!\n");
    // if deriv == 0, then leave it as zero.
  }
  else
    deriv /= _area_pp;

  return _test[_i][_qp] * deriv * phi;
}
