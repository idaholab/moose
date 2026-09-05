//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SBMBase.h"

template <typename Parent>
InputParameters
SBMBase<Parent>::validParams()
{
  InputParameters params = Parent::validParams();
  params.addParam<UserObjectName>(
      "sbm_distance_uo",
      "UserObject that provides signed distance and normal vector calculations.");

  return params;
}

template <typename Parent>
SBMBase<Parent>::SBMBase(const InputParameters & parameters)
  : Parent(parameters), _sbm_distance_uo(nullptr)
{
}

template <typename Parent>
void
SBMBase<Parent>::initialSetup()
{
  if (perform_shifted())
  {
    if (!this->isParamSetByUser("sbm_distance_uo"))
      mooseError("SBMBase: 'sbm_distance_uo' must be set when using shifted integration.");
    else
      _sbm_distance_uo =
          &static_cast<Parent &>(*this).template getUserObject<BoundaryShortestDistanceToSurface>(
              "sbm_distance_uo");
  }
}

template <typename Parent>
Real
SBMBase<Parent>::h() const
{
  return std::pow(this->_current_elem->volume(), 1.0 / this->_mesh.dimension());
}

template <typename Parent>
const RealVectorValue
SBMBase<Parent>::surrogateDistance() const
{
  const auto elem_side = std::make_pair(this->_current_elem->id(), this->_current_side);
  return _sbm_distance_uo->surrogateDistance(elem_side, this->_qp);
}

template <typename Parent>
const RealVectorValue
SBMBase<Parent>::trueNormal() const
{
  const auto elem_side = std::make_pair(this->_current_elem->id(), this->_current_side);
  return _sbm_distance_uo->trueNormal(elem_side, this->_qp);
}

// Explicit template instantiations

template class SBMBase<IntegratedBC>;
template class SBMBase<InterfaceKernel>;
template class SBMBase<ADInterfaceKernel>;
