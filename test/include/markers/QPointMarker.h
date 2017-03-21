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

#ifndef QPOINTMARKER_H
#define QPOINTMARKER_H

#include "QuadraturePointMarker.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

class QPointMarker;

template <>
InputParameters validParams<QPointMarker>();

/**
 * Simple Marker for testing q_qpoint in Markers.
 */
class QPointMarker : public QuadraturePointMarker
{
public:
  QPointMarker(const InputParameters & parameters);
  virtual ~QPointMarker(){};

protected:
  virtual MarkerValue computeQpMarker();
};

#endif /* QPOINTMARKER_H */
