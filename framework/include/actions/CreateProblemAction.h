/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef CREATEPROBLEMACTION_H
#define CREATEPROBLEMACTION_H

#include "MooseObjectAction.h"

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
