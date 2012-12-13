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

#include <map>
#include <string>
#include <vector>
#include "libmesh_common.h"

/**
 * Class to hold information about executioner time periods
 */
class TimePeriod
{
public:
  TimePeriod(const std::string & name, Real start);

  void addActiveObjects(std::string kind, const std::vector<std::string> & object_list);

  void addInactiveObjects(std::string kind, const std::vector<std::string> & object_list);

  /**
   * This method returns the list of active or inactive kernels for this time
   * period.  A Boolean flag is also written to indicate which kind of list was
   * returned.
   * @param kind - The "kind" of list being requested
   * @param is_active - A reference to a boolean that will be set to indicate whether the list
   *                    is an active or inactive set of objects
   * @return - A std::vector of active/inactive objects
   */
  const std::vector<std::string> & getObjectList(const std::string & kind, bool & is_active);

  /**
   * Returns the start time for this time period
   * @return - A real indicating the start time
   */
  Real start() const;

  /**
   * @return - The name of this time period
   */
  const std::string & name() const;

private:
  /// The name of the time period
  std::string _name;

  /// Time when this period starts (included)
  Real _start;

  /// A map of list of objects (either active or inactive)
  std::map<std::string, std::vector<std::string> > _objects;

  /// A map indicating whether the associated object list is active (true) or inactive (false)
  std::map<std::string, bool> _list_type;
};
