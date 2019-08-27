//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseVariableFE.h"

class Assembly;

class MooseVariableConstMonomial : public MooseVariableFE<Real>
{
public:
  MooseVariableConstMonomial(unsigned int var_num,
                             const FEType & fe_type,
                             SystemBase & sys,
                             Assembly & assembly,
                             Moose::VarKindType var_kind,
                             THREAD_ID tid);

  virtual void computeElemValues() override;
  virtual void computeElemValuesFace() override;
  virtual void computeNeighborValuesFace() override;
  virtual void computeNeighborValues() override;
};
