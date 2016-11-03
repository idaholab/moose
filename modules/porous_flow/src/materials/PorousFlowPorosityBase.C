/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPorosityBase.h"

template<>
InputParameters validParams<PorousFlowPorosityBase>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
  params.addClassDescription("Base class Material for porosity");
  return params;
}

PorousFlowPorosityBase::PorousFlowPorosityBase(const InputParameters & parameters) :
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
