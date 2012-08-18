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

#ifndef BOXMARKER_H
#define BOXMARKER_H

#include "Marker.h"

// libmesh includes
#include "mesh_tools.h"

class BoxMarker;

template<>
InputParameters validParams<BoxMarker>();

class BoxMarker : public Marker
{
public:
  BoxMarker(const std::string & name, InputParameters parameters);
  virtual ~BoxMarker(){};

protected:
  virtual MarkerValue computeElementMarker();

  MarkerValue _inside;
  MarkerValue _outside;

  MeshTools::BoundingBox _bounding_box;
};

#endif /* BOXMARKER_H */
