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

#include "Action.h"

class CreateProblemAction;

template<>
InputParameters validParams<CreateProblemAction>();

class CreateProblemAction : public Action
{
public:
  CreateProblemAction(const std::string & name, InputParameters parameters);

  virtual void act();

protected:
  std::string _type;
  std::string _problem_name;
  std::vector<SubdomainName> _blocks;
  std::vector<std::string> _coord_sys;
  bool _fe_cache;
};

#endif /* CREATEPROBLEMACTION_H */
