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

#include "MooseTypes.h"

class MultiApp;
class TransientMultiApp;

/**
 * Holds MultiApps and provides some services
 */
class MultiAppWarehouse
{
public:
  MultiAppWarehouse();
  virtual ~MultiAppWarehouse();

  /**
   * Get list of all MultiApps
   * @return The list of all active MultiApps
   */
  const std::vector<MultiApp *> & all() { return _all_multi_apps; }

  /**
   * Get list of all TransientMultiApps
   * @return The list of all active TransientMultiApps
   */
  const std::vector<TransientMultiApp *> & transient() { return _transient_multi_apps; }

  /**
   * Add a MultiApps
   * @param multi_app MultiApp being added
   */
  void addMultiApp(MultiApp * multi_app);

protected:
  std::vector<MultiApp *> _all_multi_apps;
  std::vector<TransientMultiApp *> _transient_multi_apps;
};

#endif // MULTIAPPWAREHOUSE_H
