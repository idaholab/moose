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

MultiAppWarehouse::MultiAppWarehouse()
{
}

MultiAppWarehouse::~MultiAppWarehouse()
{
  for (std::vector<MultiApp *>::const_iterator i = _all_multi_apps.begin(); i != _all_multi_apps.end(); ++i)
    delete *i;

}

void
MultiAppWarehouse::addMultiApp(MultiApp * multi_app)
{
  _all_multi_apps.push_back(multi_app);

  TransientMultiApp * trans_multi_app = dynamic_cast<TransientMultiApp *>(multi_app);

  if(trans_multi_app)
    _transient_multi_apps.push_back(trans_multi_app);
}
