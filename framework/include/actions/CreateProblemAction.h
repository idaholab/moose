//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CREATEPROBLEMACTION_H
#define CREATEPROBLEMACTION_H

// MOOSE includes
#include "MooseObjectAction.h"
#include "MultiMooseEnum.h"

class CreateProblemAction;

template <>
InputParameters validParams<CreateProblemAction>();

class CreateProblemAction : public MooseObjectAction
{
public:
  CreateProblemAction(InputParameters parameters);

  virtual void act() override;

protected:
  std::vector<SubdomainName> _blocks;
  MultiMooseEnum _coord_sys;
  /// One entry of coord system per block, the size of _blocks and _coord_sys has to match, except:
  /// 1. _blocks.size() == 0, then there needs to be just one entry in _coord_sys, which will
  ///    be set for the whole domain
  /// 2. _blocks.size() > 0 and no coordinate system was specified, then the whole domain will be XYZ.
  /// 3. _blocks.size() > 0 and one coordinate system was specified, then the whole domain will be that system.
  bool _fe_cache;
};

#endif /* CREATEPROBLEMACTION_H */
