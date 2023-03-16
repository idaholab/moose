//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPermeabilityBase.h"

template <bool is_ad>
InputParameters
PorousFlowPermeabilityBaseTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.addClassDescription("Base class for material permeability");
  params.addPrivateParam<std::string>("pf_material_type", "permeability");
  params.set<bool>("at_nodes") = false;
  return params;
}

template <bool is_ad>
PorousFlowPermeabilityBaseTempl<is_ad>::PorousFlowPermeabilityBaseTempl(
    const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _permeability_qp(declareGenericProperty<RealTensorValue, is_ad>("PorousFlow_permeability_qp")),
    _dpermeability_qp_dvar(
        is_ad ? nullptr
              : &declareProperty<std::vector<RealTensorValue>>("dPorousFlow_permeability_qp_dvar")),
    _dpermeability_qp_dgradvar(is_ad ? nullptr
                                     : &declareProperty<std::vector<std::vector<RealTensorValue>>>(
                                           "dPorousFlow_permeability_qp_dgradvar"))
{
  if (_nodal_material == true)
    mooseError("PorousFlowPermeability classes are only defined for at_nodes = false");
}

template class PorousFlowPermeabilityBaseTempl<false>;
template class PorousFlowPermeabilityBaseTempl<true>;
