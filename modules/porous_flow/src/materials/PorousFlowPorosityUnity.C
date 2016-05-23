/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPorosityUnity.h"

template<>
InputParameters validParams<PorousFlowPorosityUnity>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
  params.addClassDescription("This Material calculates the porosity assuming it is equal to 1.0");
  return params;
}

PorousFlowPorosityUnity::PorousFlowPorosityUnity(const InputParameters & parameters) :
    PorousFlowMaterialVectorBase(parameters),
    _porosity_nodal(declareProperty<Real>("PorousFlow_porosity_nodal")),
    _porosity_nodal_old(declarePropertyOld<Real>("PorousFlow_porosity_nodal")),
    _dporosity_nodal_dvar(declareProperty<std::vector<Real> >("dPorousFlow_porosity_nodal_dvar")),
    _dporosity_nodal_dgradvar(declareProperty<std::vector<RealGradient> >("dPorousFlow_porosity_nodal_dgradvar")),
    _porosity_qp(declareProperty<Real>("PorousFlow_porosity_qp")),
    _porosity_qp_old(declarePropertyOld<Real>("PorousFlow_porosity_qp")),
    _dporosity_qp_dvar(declareProperty<std::vector<Real> >("dPorousFlow_porosity_qp_dvar")),
    _dporosity_qp_dgradvar(declareProperty<std::vector<RealGradient> >("dPorousFlow_porosity_qp_dgradvar"))
{
}

void
PorousFlowPorosityUnity::initQpStatefulProperties()
{
  _porosity_nodal[_qp] = 1.0;
  _porosity_qp[_qp] = 1.0;

  _dporosity_nodal_dvar[_qp].assign(_num_var, 0.0);
  _dporosity_qp_dvar[_qp].assign(_num_var, 0.0);
  _dporosity_nodal_dgradvar[_qp].assign(_num_var, RealGradient());
  _dporosity_qp_dgradvar[_qp].assign(_num_var, RealGradient());
}

void
PorousFlowPorosityUnity::computeQpProperties()
{
  _porosity_nodal[_qp] = 1.0;
  _porosity_qp[_qp] = 1.0;
}
