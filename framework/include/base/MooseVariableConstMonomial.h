//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSEVARIABLECONSTMONOMIAL_H
#define MOOSEVARIABLECONSTMONOMIAL_H

#include "MooseTypes.h"
#include "MooseVariable.h"

class MooseVariableConstMonomial : public MooseVariable
{
public:
  MooseVariableConstMonomial(unsigned int var_num,
                             const FEType & fe_type,
                             SystemBase & sys,
                             Assembly & assembly,
                             Moose::VarKindType var_kind);

  virtual void computeElemValues() override;
  virtual void computeElemValuesFace() override;
  virtual void computeNeighborValuesFace() override;
  virtual void computeNeighborValues() override;

  virtual void computeElemValuesHelper(const unsigned & nqp, const Real & phi);
  virtual void computeNeighborValuesHelper(const unsigned & nqp, const Real & phi);
};

#endif /* MOOSEVARIABLECONSTMONOMIAL_H */
