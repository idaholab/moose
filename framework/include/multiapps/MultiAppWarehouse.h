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

#include "ExecuteMooseObjectWarehouse.h"
#include "MooseTypes.h"

class MultiApp;
class TransientMultiApp;

/**
 * Typedef for registered Object iterator
 */
typedef std::vector<MooseSharedPointer<MultiApp> >::const_iterator MultiAppIter;

/**
 * Holds MultiApps and provides some services
 */
class MultiAppWarehouse : public ExecuteMooseObjectWarehouse<MultiApp>
{
public:
  MultiAppWarehouse();
  virtual ~MultiAppWarehouse();

  /**
   * Get list of all TransientMultiApps
   * @return The list of all active TransientMultiApps
   */
  const MooseObjectStorage<TransientMultiApp> & getTransientStorage(ExecFlagType exec_type) const { return _transient_multi_apps[exec_type]; }

  /**
   * Add a MultiApps
   * @param multi_app MultiApp being added
   */
  void addObject(MooseSharedPointer<MultiApp> object, THREAD_ID tid = 0);

  /**
   * Gets called when the output position has changed for the parent app.
   * This will cause all MultiApps that are being output in position to change their output
   * positions as well.
   */
  void parentOutputPositionChanged();


private:

  /// Storage for TransientMultiApps (needed for calling computeDT() method)
  ExecuteMooseObjectStorage<TransientMultiApp> _transient_multi_apps;
};

#endif // MULTIAPPWAREHOUSE_H
