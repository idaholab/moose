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

#ifndef GLOBALPARAMSACTION_H
#define GLOBALPARAMSACTION_H

#include "Action.h"

class GlobalParamsAction;

template<>
InputParameters validParams<GlobalParamsAction>();


class GlobalParamsAction: public Action
{
public:
  GlobalParamsAction(const std::string & name, InputParameters params);

  virtual void act();

  template <typename T>
  inline
  T & setScalarParam(const std::string &name)
  {
    return parameters().set<T>(name);
  }

  template <typename T>
  inline
  std::vector<T> & setVectorParam(const std::string &name)
  {
    return parameters().set<std::vector<T> >(name);
  }
};
#endif //GLOBALPARAMSACTION_H
