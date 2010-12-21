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

#include "GeometricSearchInterface.h"

#include "GeometricSearchData.h"
#include "MooseSystem.h"

GeometricSearchInterface::GeometricSearchInterface(GeometricSearchData & geometric_search_data):
  _geometric_search_data(geometric_search_data)
{
}

PenetrationLocator &
GeometricSearchInterface::getPenetrationLocator(unsigned int master, unsigned int slave)
{
  return _geometric_search_data.getPenetrationLocator(master, slave);
}

NearestNodeLocator &
GeometricSearchInterface::getNearestNodeLocator(unsigned int master, unsigned int slave)
{
  return _geometric_search_data.getNearestNodeLocator(master, slave);
}
