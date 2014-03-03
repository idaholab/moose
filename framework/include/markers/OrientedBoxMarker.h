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

#ifndef ORIENTEDBOXMARKER_H
#define ORIENTEDBOXMARKER_H

#include "Marker.h"

/* libmesh includes
 * for _bounding_box
 */
#include "libmesh/mesh_tools.h"

class OrientedBoxMarker;

template<>
InputParameters validParams<OrientedBoxMarker>();

/**
 * Creates a box of specified width, length and height,
 * with its center at specified position,
 * and with the direction along the width direction specified,
 * and with the direction along the length direction specified.
 * Then elements are marked as inside or outside this box
 */
class OrientedBoxMarker : public Marker
{
public:
  OrientedBoxMarker(const std::string & name, InputParameters parameters);
  virtual ~OrientedBoxMarker(){};

protected:
  virtual MarkerValue computeElementMarker();

  Real _xmax;
  Real _ymax;
  Real _zmax;
  RealVectorValue _bottom_left;
  RealVectorValue _top_right;

  MeshTools::BoundingBox _bounding_box;

  Point _center;
  RealVectorValue _w;
  RealVectorValue _l;

  MarkerValue _inside;
  MarkerValue _outside;

  RealTensorValue _rot_matrix;

};

#endif /* ORIENTEDBOXMARKER_H */
