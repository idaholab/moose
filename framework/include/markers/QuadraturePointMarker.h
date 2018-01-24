//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef QUADRATUREPOINTMARKER_H
#define QUADRATUREPOINTMARKER_H

#include "Marker.h"
#include "Coupleable.h"
#include "MaterialPropertyInterface.h"

class QuadraturePointMarker;

template <>
InputParameters validParams<QuadraturePointMarker>();

class QuadraturePointMarker : public Marker, public Coupleable, public MaterialPropertyInterface
{
public:
  QuadraturePointMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  /**
   * Override this to compute a marker value at each quadrature point.
   *
   * The ultimate value for the element will be taken as the maximum
   * (most conservative) value for all quadrature points on the element.
   *
   * @return The MarkerValue at one quadrature point.
   */
  virtual MarkerValue computeQpMarker() = 0;

  /// The quadrature rule for the system
  QBase *& _qrule;

  /// Position of the current quadrature point
  const MooseArray<Point> & _q_point;

  /// The current quadrature point
  unsigned int _qp;
};

#endif /* QUADRATUREPOINTMARKER_H */
