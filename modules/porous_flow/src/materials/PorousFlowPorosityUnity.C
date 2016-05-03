/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPorosityUnity.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowPorosityUnity>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates the porosity assuming it is equal to 1.0");
  return params;
}

PorousFlowPorosityUnity::PorousFlowPorosityUnity(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _dictator_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
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

  const unsigned int num_var = _dictator_UO.numVariables();
  _dporosity_nodal_dvar[_qp].resize(num_var, 0.0);
  _dporosity_qp_dvar[_qp].resize(num_var, 0.0);
  _dporosity_nodal_dgradvar[_qp].resize(num_var, RealGradient());
  _dporosity_qp_dgradvar[_qp].resize(num_var, RealGradient());
}

void
PorousFlowPorosityUnity::computeQpProperties()
{
  _porosity_nodal[_qp] = 1.0;
  _porosity_qp[_qp] = 1.0;
}

