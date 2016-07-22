/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowPiecewiseLinearSink.h"

template<>
InputParameters validParams<PorousFlowPiecewiseLinearSink>()
{
  InputParameters params = validParams<PorousFlowSink>();
  params.addRequiredParam<std::vector<Real> >("pressures", "Tuple of pressure values (for the fluid_phase specified).  Must be monotonically increasing.");
  params.addRequiredParam<std::vector<Real> >("multipliers", "Tuple of multiplying values.  The flux values are multiplied by these.");
  return params;
}

PorousFlowPiecewiseLinearSink::PorousFlowPiecewiseLinearSink(const InputParameters & parameters) :
    PorousFlowSink(parameters),
    _sink_func(getParam<std::vector<Real> >("pressures"), getParam<std::vector<Real> >("multipliers")),
    _pp(getMaterialProperty<std::vector<Real> >("PorousFlow_porepressure_nodal")),
    _dpp_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_nodal_dvar"))
{
}

Real
PorousFlowPiecewiseLinearSink::multiplier()
{
  return PorousFlowSink::multiplier() * _sink_func.sample(_pp[_qp_map[_i]][_ph]);
}

Real
PorousFlowPiecewiseLinearSink::dmultiplier_dvar(unsigned int pvar)
{
  return PorousFlowSink::dmultiplier_dvar(pvar) * _sink_func.sample(_pp[_qp_map[_i]][_ph]) + PorousFlowSink::multiplier() * _sink_func.sampleDerivative(_pp[_qp_map[_i]][_ph]) * _dpp_dvar[_qp_map[_i]][_ph][pvar];
}
