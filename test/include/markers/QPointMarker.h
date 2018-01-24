//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef QPOINTMARKER_H
#define QPOINTMARKER_H

#include "QuadraturePointMarker.h"

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
