/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPolyLineSink.h"

template<>
InputParameters validParams<PorousFlowPolyLineSink>()
{
  InputParameters params = validParams<PorousFlowLineSink>();
  params.addRequiredParam<std::vector<Real> >("p_or_t_vals", "Tuple of pressure (or temperature) values.  Must be monotonically increasing.");
  params.addRequiredParam<std::vector<Real> >("fluxes", "Tuple of flux values (measured in kg.m^-1.s^-1 if no 'use_*' are employed).  These flux values are multiplied by the line-segment length to achieve a flux in kg.s^-1.  A piecewise-linear fit is performed to the (p_or_t_vals,flux) pairs to obtain the flux at any arbitrary pressure (or temperature).  If a quad-point pressure is less than the first pressure value, the first flux value is used.  If quad-point pressure exceeds the final pressure value, the final flux value is used.  This flux is OUT of the medium: hence positive values of flux means this will be a SINK, while negative values indicate this flux will be a SOURCE.");
  params.addClassDescription("Approximates a polyline sink by using a number of point sinks with given weighting whose positions are read from a file");
  return params;
}

PorousFlowPolyLineSink::PorousFlowPolyLineSink(const InputParameters & parameters) :
    PorousFlowLineSink(parameters),
    _sink_func(getParam<std::vector<Real> >("p_or_t_vals"), getParam<std::vector<Real> >("fluxes"))
{
}

Real
PorousFlowPolyLineSink::computeQpBaseOutflow(unsigned current_dirac_ptid) const
{
  Real outflow = 0.0;

  if (current_dirac_ptid > 0)
    // contribution from half-segment "behind" this point (must have >1 point for current_dirac_ptid>0)
    outflow += _half_seg_len[current_dirac_ptid - 1];

  if (current_dirac_ptid + 1 < _zs.size() || _zs.size() == 1)
    // contribution from half-segment "ahead of" this point, or we only have one point
    outflow += _half_seg_len[current_dirac_ptid];

  return outflow * _test[_i][_qp] * _sink_func.sample(ptqp()) * _rs[current_dirac_ptid];
}

void
PorousFlowPolyLineSink::computeQpBaseOutflowJacobian(unsigned jvar, unsigned current_dirac_ptid, Real & outflow, Real & outflowp) const
{
  outflow = 0.0;
  outflowp = 0.0;

  if (_dictator.notPorousFlowVariable(jvar))
    return;
  const unsigned pvar = _dictator.porousFlowVariableNum(jvar);

  if (current_dirac_ptid > 0)
    // contribution from half-segment "behind" this point (must have >1 point for current_dirac_ptid>0)
    outflow += _half_seg_len[current_dirac_ptid - 1];

  if (current_dirac_ptid + 1 < _zs.size() || _zs.size() == 1)
    // contribution from half-segment "ahead of" this point, or we only have one point
    outflow += _half_seg_len[current_dirac_ptid];

  outflowp = outflow * _test[_i][_qp] * _sink_func.sampleDerivative(ptqp()) * dptqp(pvar) * _phi[_j][_qp] * _rs[current_dirac_ptid];
  outflow *= _test[_i][_qp] * _sink_func.sample(ptqp()) * _rs[current_dirac_ptid];
}
