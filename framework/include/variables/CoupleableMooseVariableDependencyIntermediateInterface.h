//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Coupleable.h"
#include "ScalarCoupleable.h"
#include "MooseVariableDependencyInterface.h"

/**
 * Intermediate base class that ties together all the interfaces for getting
 * MooseVariableFEBases with the MooseVariableDependencyInterface
 */
class CoupleableMooseVariableDependencyIntermediateInterface
  : public Coupleable,
    public ScalarCoupleable,
    public MooseVariableDependencyInterface
{
public:
  CoupleableMooseVariableDependencyIntermediateInterface(const MooseObject * moose_object,
                                                         bool nodal,
                                                         bool is_fv = false);
};
