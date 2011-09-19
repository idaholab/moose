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

#ifndef MOOSEOBJECTACTION_H
#define MOOSEOBJECTACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class MooseObjectAction;

template<>
InputParameters validParams<MooseObjectAction>();


class MooseObjectAction : public Action
{
public:
  MooseObjectAction(const std::string & name, InputParameters params);

  virtual void act() = 0;

  inline InputParameters & getMooseObjectParams() { return _moose_object_pars; }

  virtual void addParamsPtrs(std::vector<InputParameters *> & param_ptrs);

protected:
  std::string _type;
  InputParameters _moose_object_pars;
};

#endif // MOOSEOBJECTACTION_H
