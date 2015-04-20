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

#ifndef MULTIAPPWAREHOUSE_H
#define MULTIAPPWAREHOUSE_H

#include <vector>
#include <map>
#include <set>

#include "Warehouse.h"

class MultiApp;
class TransientMultiApp;

/**
 * Holds MultiApps and provides some services
 */
class MultiAppWarehouse : public Warehouse<MultiApp>
{
public:
  MultiAppWarehouse();
  virtual ~MultiAppWarehouse();

  /**
   * Get list of all TransientMultiApps
   * @return The list of all active TransientMultiApps
   */
  const std::vector<TransientMultiApp *> & transient() const { return _transient_multi_apps; }

  /**
   * Add a MultiApps
   * @param multi_app MultiApp being added
   */
  void addMultiApp(MooseSharedPointer<MultiApp> multi_app);

  /**
   * Whether or not this warehouse has a MultiApp named multi_app_name
   * @param multi_app_name The name of the MultiApp we're looking for
   * @return True if that MultiApp exists False otherwise
   */
  bool hasMultiApp(const std::string & multi_app_name) const;

  /**
   * Returns whether there are any multiapps
   */
  bool hasMultiApp() const;

  /**
   * Get a MultiApp by name.  Will error if the MultiApp doesn't exist in this Warehouse.
   * @param multi_app_name The name of the MultiApp to get.
   * @return A pointer to the MultiApp
   */
  MultiApp * getMultiApp(const std::string & multi_app_name) const;

  /**
   * Gets called when the output position has changed for the parent app.
   * This will cause all MultiApps that are being output in position to change their output
   * positions as well.
   */
  void parentOutputPositionChanged();

  /**
   * Calls the initialSetup() function for all Multiapps in the Warehouse
   */
  void initialSetup();

protected:
  std::vector<TransientMultiApp *> _transient_multi_apps;

private:
  /// Hold shared pointers for automatic cleanup
  std::vector<MooseSharedPointer<MultiApp> > _all_ptrs;
};

#endif // MULTIAPPWAREHOUSE_H
