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

#include "GeometricSearchData.h"

//Moose includes
#include "MooseSystem.h"

GeometricSearchData::GeometricSearchData(MooseSystem & moose_system, Mesh * & mesh)
  :_moose_system(moose_system),
   _mesh(mesh)
{}


PenetrationLocator &
GeometricSearchData::getPenetrationLocator(unsigned int master, unsigned int slave)
{
  PenetrationLocator * pl = _penetration_locators[std::pair<unsigned int, unsigned int>(master, slave)];

  if(!pl)
  {
    pl = new PenetrationLocator(_moose_system, *_mesh, master, slave);
    _penetration_locators[std::pair<unsigned int, unsigned int>(master, slave)] = pl;
  }
  
  return *pl;
}

