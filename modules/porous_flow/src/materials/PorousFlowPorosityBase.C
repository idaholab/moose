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
  params.addParam<bool>("strain_at_nearest_qp", false, "When calculating nodal porosity that depends on strain, use the strain at the nearest quadpoint.  This adds a small extra computational burden, and is not necessary for simulations involving only linear lagrange elements.  If you set this to true, you will also want to set the same parameter to true for related Kernels and Materials");
  params.addClassDescription("Base class Material for porosity");
  return params;
}

PorousFlowPorosityBase::PorousFlowPorosityBase(const InputParameters & parameters) :
    PorousFlowMaterialVectorBase(parameters),
    _strain_at_nearest_qp(getParam<bool>("strain_at_nearest_qp")),
    _porosity(_nodal_material ? declareProperty<Real>("PorousFlow_porosity_nodal") : declareProperty<Real>("PorousFlow_porosity_qp")),
    _porosity_old(_nodal_material ? declarePropertyOld<Real>("PorousFlow_porosity_nodal") : declarePropertyOld<Real>("PorousFlow_porosity_qp")),
    _dporosity_dvar(_nodal_material ? declareProperty<std::vector<Real> >("dPorousFlow_porosity_nodal_dvar") : declareProperty<std::vector<Real> >("dPorousFlow_porosity_qp_dvar")),
    _dporosity_dgradvar(_nodal_material ? declareProperty<std::vector<RealGradient> >("dPorousFlow_porosity_nodal_dgradvar") : declareProperty<std::vector<RealGradient> >("dPorousFlow_porosity_qp_dgradvar"))
{
}
