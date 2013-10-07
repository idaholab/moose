#ifndef INITIALCONDITIONWAREHOUSE_H
#define INITIALCONDITIONWAREHOUSE_H

#include <vector>
#include <map>
#include <string>
#include "MooseTypes.h"

class InitialCondition;
class ScalarInitialCondition;

/**
 * Warehouse for storing initial conditions
 */
class InitialConditionWarehouse
{
public:
  InitialConditionWarehouse();
  virtual ~InitialConditionWarehouse();

  /**
   * Initial setup
   */
  void initialSetup();

  /**
   * Add an initial condition for field variable
   * @param var_name The variable name this initial condition works on
   * @param ic The initial condition object
   */
  void addInitialCondition(const std::string & var_name, SubdomainID blockid, InitialCondition * ic);

  /**
   * Get an initial condition
   * @param var_name The name of the variable for which we are retrieving the initial condition
   * @return The initial condition object if the initial condition exists, NULL otherwise
   */
  InitialCondition * getInitialCondition(const std::string & var_name, SubdomainID blockid);

  /**
   * Add an initial condition for scalar variable
   * @param var_name The variable name this initial condition works on
   * @param ic The initial condition object
   */
  void addScalarInitialCondition(const std::string & var_name, ScalarInitialCondition * ic);

  /**
   * Get a scalar initial condition
   * @param var_name The name of the variable for which we are retrieving the initial condition
   * @return The initial condition object if the initial condition exists, NULL otherwise
   */
  ScalarInitialCondition * getScalarInitialCondition(const std::string & var_name);

protected:
  /// Initial conditions: [var name] -> [block_id] -> initial condition (only 1 IC per sub-block)
  std::map<std::string, std::map<SubdomainID, InitialCondition *> > _ics;

  /// Initial conditions: [var name] -> initial condition
  std::map<std::string, ScalarInitialCondition *> _scalar_ics;
};

#endif /* INITIALCONDITIONWAREHOUSE_H */
