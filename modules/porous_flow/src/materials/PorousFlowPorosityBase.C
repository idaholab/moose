//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPorosityBase.h"

template <bool is_ad>
InputParameters
PorousFlowPorosityBaseTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.addPrivateParam<std::string>("pf_material_type", "porosity");
  params.addClassDescription("Base class Material for porosity");
  return params;
}

template <bool is_ad>
PorousFlowPorosityBaseTempl<is_ad>::PorousFlowPorosityBaseTempl(const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _porosity(_nodal_material ? declareGenericProperty<Real, is_ad>("PorousFlow_porosity_nodal")
                              : declareGenericProperty<Real, is_ad>("PorousFlow_porosity_qp")),
    _dporosity_dvar(is_ad ? nullptr
                    : _nodal_material
                        ? &declareProperty<std::vector<Real>>("dPorousFlow_porosity_nodal_dvar")
                        : &declareProperty<std::vector<Real>>("dPorousFlow_porosity_qp_dvar")),
    _dporosity_dgradvar(
        is_ad ? nullptr
        : _nodal_material
            ? &declareProperty<std::vector<RealGradient>>("dPorousFlow_porosity_nodal_dgradvar")
            : &declareProperty<std::vector<RealGradient>>("dPorousFlow_porosity_qp_dgradvar"))
{
}

template class PorousFlowPorosityBaseTempl<false>;
template class PorousFlowPorosityBaseTempl<true>;
