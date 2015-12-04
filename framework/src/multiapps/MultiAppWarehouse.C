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

#include "MultiAppWarehouse.h"

#include "MultiApp.h"
#include "TransientMultiApp.h"
#include "MooseError.h"

MultiAppWarehouse::MultiAppWarehouse() :
    ExecuteMooseObjectWarehouse<MultiApp>(/*threaded=*/false)
{
}

MultiAppWarehouse::~MultiAppWarehouse()
{
}

void
MultiAppWarehouse::addObject(MooseSharedPointer<MultiApp> object, THREAD_ID /*tid*/)
{
  ExecuteMooseObjectWarehouse<MultiApp>::addObject(object);

  // Store TranseintMultiApp objects in another containter, this is needed for calling computeDT
  MooseSharedPointer<TransientMultiApp> trans_multi_app = MooseSharedNamespace::dynamic_pointer_cast<TransientMultiApp>(object);
  if (trans_multi_app)
    _transient_multi_apps.addObject(trans_multi_app);
}


void
MultiAppWarehouse::parentOutputPositionChanged()
{
  std::map<ExecFlagType, MooseObjectStorage<MultiApp> >::iterator it;
  std::vector<MooseSharedPointer<MultiApp> >::iterator jt;
  for (it = _execute_objects.begin(); it != _execute_objects.end(); ++it)
  {
    const std::vector<MooseSharedPointer<MultiApp> > & objects = it->second.getActiveObjects();
    for (std::vector<MooseSharedPointer<MultiApp> >::const_iterator jt = objects.begin(); jt != objects.end(); ++jt)
      (*jt)->parentOutputPositionChanged();
  }
}
