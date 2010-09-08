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

#ifndef INITIALCONDITIONWAREHOUSE_H
#define INITIALCONDITIONWAREHOUSE_H

#include <vector>

#include "InitialCondition.h"

/**
 * Typedef to hide implementation details
 */
typedef std::map<std::string, InitialCondition *>::iterator InitialConditionIterator;


class InitialConditionWarehouse
{
public:
  InitialConditionWarehouse();
  virtual ~InitialConditionWarehouse();

  InitialConditionIterator activeInitialConditionsBegin();
  InitialConditionIterator activeInitialConditionsEnd();

  InitialCondition * getInitialCondition(const std::string & var_name);

  void addIC(const std::string & var_name, InitialCondition *ic);

protected:
  std::map<std::string, InitialCondition *> _active_initial_conditions;
};

#endif // INITIALCONDITIONWAREHOUSE_H
