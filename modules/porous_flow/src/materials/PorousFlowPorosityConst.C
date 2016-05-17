/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPorosityConst.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowPorosityConst>()
{
  InputParameters params = validParams<PorousFlowPorosityUnity>();
  params.addRequiredParam<Real>("porosity", "The porosity, which is assumed constant for this material");
  params.addClassDescription("This Material calculates the porosity assuming it is constant");
  return params;
}

PorousFlowPorosityConst::PorousFlowPorosityConst(const InputParameters & parameters) :
    PorousFlowPorosityUnity(parameters),
    _input_porosity(getParam<Real>("porosity"))
{
}

void
PorousFlowPorosityConst::initQpStatefulProperties()
{
  _porosity_nodal[_qp] = _input_porosity; // this becomes _porosity_old[_qp] in the first call to computeQpProperties
  _porosity_qp[_qp] = _input_porosity; // this becomes _porosity_old[_qp] in the first call to computeQpProperties

  const unsigned int num_var = _dictator.numVariables();
  _dporosity_nodal_dvar[_qp].assign(num_var, 0.0);
  _dporosity_qp_dvar[_qp].assign(num_var, 0.0);
  _dporosity_nodal_dgradvar[_qp].assign(num_var, RealGradient());
  _dporosity_qp_dgradvar[_qp].assign(num_var, RealGradient());
}

void
PorousFlowPorosityConst::computeQpProperties()
{
  _porosity_nodal[_qp] = _input_porosity;
  _porosity_qp[_qp] = _input_porosity;
}

