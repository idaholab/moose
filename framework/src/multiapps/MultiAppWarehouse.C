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

MultiAppWarehouse::MultiAppWarehouse()
{
}

MultiAppWarehouse::~MultiAppWarehouse()
{
}

void
MultiAppWarehouse::addMultiApp(MooseSharedPointer<MultiApp> multi_app)
{
  _all_ptrs.push_back(multi_app);
  _all_objects.push_back(multi_app.get());

  MooseSharedPointer<TransientMultiApp> trans_multi_app = MooseSharedNamespace::dynamic_pointer_cast<TransientMultiApp>(multi_app);

  if (trans_multi_app.get())
    _transient_multi_apps.push_back(trans_multi_app.get());
}

bool
MultiAppWarehouse::hasMultiApp(const std::string & multi_app_name) const
{
  for (std::vector<MultiApp *>::const_iterator i = _all_objects.begin(); i != _all_objects.end(); ++i)
    if ((*i)->name() == multi_app_name)
      return true;

  return false;
}

bool
MultiAppWarehouse::hasMultiApp() const
{
  return !_all_objects.empty();
}

MultiApp *
MultiAppWarehouse::getMultiApp(const std::string & multi_app_name) const
{
  for (std::vector<MultiApp *>::const_iterator i = _all_objects.begin(); i != _all_objects.end(); ++i)
    if ((*i)->name() == multi_app_name)
      return *i;

  mooseError("Unknown MultiApp: " << multi_app_name);
}

void
MultiAppWarehouse::parentOutputPositionChanged()
{
  for (unsigned int i=0; i<_all_objects.size(); i++)
    _all_objects[i]->parentOutputPositionChanged();
}

void
MultiAppWarehouse::initialSetup()
{
  for (std::vector<MultiApp *>::iterator i = _all_objects.begin(); i != _all_objects.end(); ++i)
    (*i)->initialSetup();
}
