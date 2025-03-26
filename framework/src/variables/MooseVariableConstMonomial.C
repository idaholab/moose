//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableConstMonomial.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "Assembly.h"

#include "libmesh/quadrature.h"
#include "libmesh/enum_to_string.h"

registerMooseObject("MooseApp", MooseVariableConstMonomial);

InputParameters
MooseVariableConstMonomial::validParams()
{
  auto params = MooseVariableBase::validParams();
  params.addClassDescription("Specialization for constant monomials that avoids unnecessary loops");
  params.set<MooseEnum>("family") = "MONOMIAL";
  params.set<MooseEnum>("order") = "CONSTANT";
  return params;
}

MooseVariableConstMonomial::MooseVariableConstMonomial(const InputParameters & parameters)
  : MooseVariable(parameters)
{
  if (_fe_type.order != CONSTANT || _fe_type.family != MONOMIAL)
    mooseError("This type is only meant for a CONSTANT "
               "MONOMIAL finite element basis. You have requested a ",
               Utility::enum_to_string(_fe_type.family),
               " family and order ",
               Utility::enum_to_string(Order(_fe_type.order)));
}

void
MooseVariableConstMonomial::computeElemValues()
{
  _element_data->setGeometry(Moose::Volume);
  _element_data->computeMonomialValues();
}

void
MooseVariableConstMonomial::computeElemValuesFace()
{
  _element_data->setGeometry(Moose::Face);
  _element_data->computeMonomialValues();
}

void
MooseVariableConstMonomial::computeNeighborValues()
{
  _neighbor_data->setGeometry(Moose::Volume);
  _neighbor_data->computeMonomialValues();
}

void
MooseVariableConstMonomial::computeNeighborValuesFace()
{
  _neighbor_data->setGeometry(Moose::Face);
  _neighbor_data->computeMonomialValues();
}
