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

#ifndef GEOMETRICSEARCHDATA_H
#define GEOMETRICSEARCHDATA_H

//libmesh includes
#include "mesh.h"

#include <map>

//Forward Declarations
class PenetrationLocator;
class MooseSystem;

class GeometricSearchData
{
public:
  GeometricSearchData(MooseSystem & moose_system, Mesh * & mesh);

  PenetrationLocator & getPenetrationLocator(unsigned int master, unsigned int slave);

  MooseSystem & _moose_system;
  Mesh * & _mesh;
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> _penetration_locators;
};
#endif //GEOMETRICSEARCHDATA_H
