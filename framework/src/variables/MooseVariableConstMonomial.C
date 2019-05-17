//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

MooseVariableConstMonomial::MooseVariableConstMonomial(unsigned int var_num,
                                                       const FEType & fe_type,
                                                       SystemBase & sys,
                                                       Assembly & assembly,
                                                       Moose::VarKindType var_kind,
                                                       THREAD_ID tid)
  : MooseVariable(var_num, fe_type, sys, assembly, var_kind, tid)
{
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
