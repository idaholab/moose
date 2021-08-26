//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableField.h"

template <typename OutputType>
InputParameters
MooseVariableField<OutputType>::validParams()
{
  return MooseVariableFieldBase::validParams();
}

template <typename OutputType>
MooseVariableField<OutputType>::MooseVariableField(const InputParameters & parameters)
  : MooseVariableFieldBase(parameters)
{
}

template <typename OutputType>
typename Moose::ADType<OutputType>::type
MooseVariableField<OutputType>::evaluate(const QpArg & qp, unsigned int state) const
{
  mooseAssert(this->hasBlocks(std::get<0>(qp)->subdomain_id()),
              "This variable doesn't exist in the requested block!");

  switch (state)
  {
    case 0:
      return adSln()[std::get<1>(qp)];

    case 1:
      return slnOld()[std::get<1>(qp)];

    case 2:
      return slnOlder()[std::get<1>(qp)];

    default:
      mooseError("Unsupported state ", state, " in MooseVariableField::evaluate");
  }
}

template <typename OutputType>
typename Moose::ADType<OutputType>::type
MooseVariableField<OutputType>::evaluate(
    const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp, unsigned int state) const
{
  mooseAssert(this->hasBlocks(std::get<2>(tqp)),
              "This variable doesn't exist in the requested block!");
  const auto elem_type = std::get<0>(tqp);
  const auto qp = std::get<1>(tqp);
  switch (elem_type)
  {
    case Moose::ElementType::Element:
    {
      switch (state)
      {
        case 0:
          return adSln()[qp];

        case 1:
          return slnOld()[qp];

        case 2:
          return slnOlder()[qp];

        default:
          mooseError("Unsupported state ", state, " in MooseVariableField::evaluate");
      }
    }

    case Moose::ElementType::Neighbor:
    {
      switch (state)
      {
        case 0:
          return adSlnNeighbor()[qp];

        case 1:
          return slnOldNeighbor()[qp];

        default:
          mooseError("Unsupported state ", state, " in MooseVariableField::evaluate");
      }
    }

    default:
      mooseError("Unrecognized element type");
  }
}

template class MooseVariableField<Real>;
template class MooseVariableField<RealVectorValue>;
template class MooseVariableField<RealEigenVector>;
