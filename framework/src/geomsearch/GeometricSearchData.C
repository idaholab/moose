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
#include "NearestNodeLocator.h"

GeometricSearchData::GeometricSearchData(MooseSystem & moose_system, Mesh * & mesh)
  :_moose_system(moose_system),
   _mesh(mesh)
{}

void
GeometricSearchData::update()
{
  std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *>::iterator nnl_it = _nearest_node_locators.begin();
  const std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *>::iterator nnl_end = _nearest_node_locators.end();

  for(; nnl_it != nnl_end; ++nnl_it)
  {
    NearestNodeLocator * nnl = nnl_it->second;

    nnl->findNodes();
  }

  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator pl_it = _penetration_locators.begin();
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator pl_end = _penetration_locators.end();

  for(; pl_it != pl_end; ++pl_it)
  {
    PenetrationLocator * pl = pl_it->second;

    pl->detectPenetration();
  }
}

PenetrationLocator &
GeometricSearchData::getPenetrationLocator(unsigned int master, unsigned int slave)
{
  PenetrationLocator * pl = _penetration_locators[std::pair<unsigned int, unsigned int>(master, slave)];

  if(!pl)
  {
    pl = new PenetrationLocator(_moose_system, *this, *_mesh, master, slave);
    _penetration_locators[std::pair<unsigned int, unsigned int>(master, slave)] = pl;
  }

  return *pl;
}

NearestNodeLocator &
GeometricSearchData::getNearestNodeLocator(unsigned int master, unsigned int slave)
{
  NearestNodeLocator * nnl = _nearest_node_locators[std::pair<unsigned int, unsigned int>(master, slave)];

  if(!nnl)
  {
    nnl = new NearestNodeLocator(_moose_system, *_mesh, master, slave);
    _nearest_node_locators[std::pair<unsigned int, unsigned int>(master, slave)] = nnl;
  }

  return *nnl;
}
