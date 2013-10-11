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
   * Update the list of active ICs
   * @param subdomain The subdomain for which we are updating the list of active ICs
   */
  void updateActiveICs(SubdomainID subdomain);

  /**
   * Get the list of active ICs
   * @return the list of active ICs
   */
  const std::vector<InitialCondition *> & active();

  /**
   * Get the list of active boundary ICs
   * @return the list of active boundary ICs
   */
  const std::vector<InitialCondition *> & activeBoundary(BoundaryID boundary_id);

  /**
   * Add an initial condition for field variable
   * @param var_name The variable name this initial condition works on
   * @param blockid The block to which this IC is restricted
   * @param ic The initial condition object
   */
  void addInitialCondition(const std::string & var_name, SubdomainID blockid, InitialCondition * ic);

  /**
   * Add an initial condition for field variable restricted to a boundary
   * @param var_name The variable name this initial condition works on
   * @param boundary_id The boundary to which this IC is restricted
   * @param ic The initial condition object
   */
  void addBoundaryInitialCondition(const std::string & var_name, BoundaryID boundary_id, InitialCondition * ic);

  // Scalar ICs //////

  /**
   * Get the list of active scalar ICs
   * @return the list of active scalar ICs
   */
  const std::vector<ScalarInitialCondition *> & activeScalar();

  /**
   * Add an initial condition for scalar variable
   * @param var_name The variable name this initial condition works on
   * @param ic The initial condition object
   */
  void addScalarInitialCondition(const std::string & var_name, ScalarInitialCondition * ic);

protected:
  /// Active ICs
  std::vector<InitialCondition *> _active_ics;
  /// All block-restricted ICs
  std::map<SubdomainID, std::vector<InitialCondition *> > _all_ics;
  /// Initial conditions: [var name] -> [block_id] -> initial condition (only 1 IC per sub-block)
  std::map<std::string, std::map<SubdomainID, InitialCondition *> > _ics;
  /// All boundary restricted ICs
  std::map<BoundaryID, std::vector<InitialCondition *> > _active_boundary_ics;
  /// Initial conditions: [var name] -> [boundary_id] -> initial condition (only 1 IC per boundary)
  std::map<std::string, std::map<BoundaryID, InitialCondition *> > _boundary_ics;

  /// Initial conditions: [var name] -> initial condition
  std::map<std::string, ScalarInitialCondition *> _scalar_ics;
  /// Active scalar ICs
  std::vector<ScalarInitialCondition *> _active_scalar_ics;

private:
  /**
   * This routine uses the Dependency Resolver to sort initial conditions based on dependencies they
   * might have on coupled values
   */
  void sortICs(std::vector<InitialCondition *> & ics);

  /**
   * This routine uses the Dependency Resolver to sort scalar initial conditions based on dependencies they
   * might have on coupled values
   */
  void sortScalarICs(std::vector<ScalarInitialCondition *> & ics);
};

#endif /* INITIALCONDITIONWAREHOUSE_H */
