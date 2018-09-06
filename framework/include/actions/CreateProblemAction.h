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

  void CreateTagVectors();

protected:
  /**
   * Method used to check the consistency of multiple CreateProblemActions that may be added to
   * the system either through the Parser or user-defined Actions. This method makes sure that
   * types match among multiple Actions and also determines which Action should take precedence when
   * multiple exist.
   */
  bool checkSanity(const std::list<Action *> &) const;

  ///@{
  /**
   * One entry of coord system per block, the size of _blocks and _coord_sys has to match, except:
   * 1. _blocks.size() == 0, then there needs to be just one entry in _coord_sys, which will
   *     be set for the whole domain.
   * 2. _blocks.size() > 0 and no coordinate system is specified, then the whole domain will be XYZ.
   * 3. _blocks.size() > 0 and one coordinate system is specified, then the whole domain will be
   *     that coordinate system.
   */
  std::vector<SubdomainName> _blocks;
  MultiMooseEnum _coord_sys;
  ///@}

  /// Boolean indicating if _this_ Action should be the one used to create the problem.
  bool _use_this_action;
};

#endif /* CREATEPROBLEMACTION_H */
