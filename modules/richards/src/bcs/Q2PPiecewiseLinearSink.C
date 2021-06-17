//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Q2PPiecewiseLinearSink.h"

// MOOSE includes
#include "MooseVariable.h"

// C++ includes
#include <iostream>

registerMooseObject("RichardsApp", Q2PPiecewiseLinearSink);

InputParameters
Q2PPiecewiseLinearSink::validParams()
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
      "fluid_density",
      "A RichardsDensity UserObject that defines the fluid density as a function of pressure.");
  params.addRequiredParam<UserObjectName>(
      "fluid_relperm",
      "A RichardsRelPerm UserObject (eg RichardsRelPermPower) that defines the "
      "fluid relative permeability as a function of the saturation Variable.");
  params.addRequiredCoupledVar("other_var",
                               "The other variable in the 2-phase system.  If "
                               "Variable=porepressure, the other_var=saturation, and "
                               "vice-versa.");
  params.addRequiredParam<bool>("var_is_porepressure",
                                "This flag is needed to correctly calculate the Jacobian entries.  "
                                "If set to true, this Sink will extract fluid from the phase with "
                                "porepressure as its Variable (usually the liquid phase).  If set "
                                "to false, this Sink will extract fluid from the phase with "
                                "saturation as its variable (usually the gas phase)");
  params.addRequiredParam<Real>("fluid_viscosity", "The fluid dynamic viscosity");
  params.addClassDescription("Sink of fluid, controlled by (pressure, bare_fluxes) interpolation.  "
                             "This is for use in Q2P models");
  return params;
}

Q2PPiecewiseLinearSink::Q2PPiecewiseLinearSink(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _use_mobility(getParam<bool>("use_mobility")),
    _use_relperm(getParam<bool>("use_relperm")),
    _sink_func(getParam<std::vector<Real>>("pressures"),
               getParam<std::vector<Real>>("bare_fluxes")),
    _m_func(getFunction("multiplying_fcn")),
    _density(getUserObject<RichardsDensity>("fluid_density")),
    _relperm(getUserObject<RichardsRelPerm>("fluid_relperm")),
    _other_var_nodal(coupledDofValues("other_var")),
    _other_var_num(coupled("other_var")),
    _var_is_pp(getParam<bool>("var_is_porepressure")),
    _viscosity(getParam<Real>("fluid_viscosity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),
    _num_nodes(0),
    _pp(0),
    _sat(0),
    _nodal_density(0),
    _dnodal_density_dp(0),
    _nodal_relperm(0),
    _dnodal_relperm_ds(0)
{
}

void
Q2PPiecewiseLinearSink::prepareNodalValues()
{
  _num_nodes = _other_var_nodal.size();

  // set _pp and _sat variables
  _pp.resize(_num_nodes);
  _sat.resize(_num_nodes);
  if (_var_is_pp)
  {
    for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
    {
      _pp[nodenum] = _var.dofValues()[nodenum];
      _sat[nodenum] = _other_var_nodal[nodenum];
    }
  }
  else
  {
    for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
    {
      _pp[nodenum] = _other_var_nodal[nodenum];
      _sat[nodenum] = _var.dofValues()[nodenum];
    }
  }

  // calculate derived things
  if (_use_mobility)
  {
    _nodal_density.resize(_num_nodes);
    _dnodal_density_dp.resize(_num_nodes);
    for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
    {
      _nodal_density[nodenum] = _density.density(_pp[nodenum]);
      _dnodal_density_dp[nodenum] = _density.ddensity(_pp[nodenum]);
    }
  }

  if (_use_relperm)
  {
    _nodal_relperm.resize(_num_nodes);
    _dnodal_relperm_ds.resize(_num_nodes);
    for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
    {
      _nodal_relperm[nodenum] = _relperm.relperm(_sat[nodenum]);
      _dnodal_relperm_ds[nodenum] = _relperm.drelperm(_sat[nodenum]);
    }
  }
}

void
Q2PPiecewiseLinearSink::computeResidual()
{
  prepareNodalValues();
  IntegratedBC::computeResidual();
}

Real
Q2PPiecewiseLinearSink::computeQpResidual()
{
  Real flux = 0;
  Real k = 0;

  flux = _test[_i][_qp] * _sink_func.sample(_pp[_i]);
  if (_use_mobility)
  {
    k = (_permeability[0] * _normals[_qp]) * _normals[_qp]; // assume that _permeability is constant
                                                            // throughout element so doesn't need to
                                                            // be upwinded
    flux *= _nodal_density[_i] * k / _viscosity;
  }
  if (_use_relperm)
    flux *= _nodal_relperm[_i];

  flux *= _m_func.value(_t, _q_point[_qp]);

  return flux;
}

void
Q2PPiecewiseLinearSink::computeJacobian()
{
  prepareNodalValues();
  IntegratedBC::computeJacobian();
}

Real
Q2PPiecewiseLinearSink::computeQpJacobian()
{
  return jac(_var.number());
}

void
Q2PPiecewiseLinearSink::computeOffDiagJacobian(const unsigned int jvar)
{
  prepareNodalValues();
  IntegratedBC::computeOffDiagJacobian(jvar);
}

Real
Q2PPiecewiseLinearSink::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number() || jvar == _other_var_num)
    return jac(jvar);

  return 0.0;
}

Real
Q2PPiecewiseLinearSink::jac(unsigned int wrt_num)
{
  Real flux = 0;
  Real deriv = 0;
  Real k = 0;
  Real mob = 0;
  Real mobp = 0;
  Real phi = 1;

  if (_i != _j)
    return 0.0; // residual at node _i only depends on variables at that node

  flux = _sink_func.sample(_pp[_i]);

  if (_var_is_pp)
  {
    // derivative of the _sink_func
    if (wrt_num == _var.number())
      deriv = _sink_func.sampleDerivative(_pp[_i]);
    else
      deriv = 0;

    // add derivative of the mobility
    if (_use_mobility)
    {
      k = (_permeability[0] * _normals[_qp]) * _normals[_qp];
      mob = _nodal_density[_i] * k / _viscosity;
      if (wrt_num == _var.number())
        mobp = _dnodal_density_dp[_i] * k / _viscosity; // else mobp = 0
      deriv = mob * deriv + mobp * flux;
      flux *= mob;
    }

    // add derivative of the relperm
    if (_use_relperm)
    {
      if (wrt_num == _other_var_num)
        deriv = _nodal_relperm[_i] * deriv + _dnodal_relperm_ds[_i] * flux;
      else
        deriv = _nodal_relperm[_i] * deriv;
    }
  }
  else
  {
    // derivative of the _sink_func
    if (wrt_num == _other_var_num)
      deriv = _sink_func.sampleDerivative(_pp[_i]);
    else
      deriv = 0;

    // add derivative of the mobility
    if (_use_mobility)
    {
      k = (_permeability[0] * _normals[_qp]) * _normals[_qp];
      mob = _nodal_density[_i] * k / _viscosity;
      if (wrt_num == _other_var_num)
        mobp = _dnodal_density_dp[_i] * k / _viscosity; // else mobp = 0
      deriv = mob * deriv + mobp * flux;
      flux *= mob;
    }

    // add derivative of the relperm
    if (_use_relperm)
    {
      if (wrt_num == _var.number())
        deriv = _nodal_relperm[_i] * deriv + _dnodal_relperm_ds[_i] * flux;
      else
        deriv = _nodal_relperm[_i] * deriv;
    }
  }

  deriv *= _m_func.value(_t, _q_point[_qp]);

  return _test[_i][_qp] * deriv * phi;
}
