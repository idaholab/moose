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

// MOOSE includes
#include "Marker.h"
#include "OrientedBoxInterface.h"

// Forward declarations
class OrientedBoxMarker;

template <>
InputParameters validParams<OrientedBoxMarker>();

/**
 * Creates a box of specified width, length and height,
 * with its center at specified position,
 * and with the direction along the width direction specified,
 * and with the direction along the length direction specified.
 * Then elements are marked as inside or outside this box
 */
class OrientedBoxMarker : public Marker, public OrientedBoxInterface
{
public:
  OrientedBoxMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  MarkerValue _inside;
  MarkerValue _outside;
};

#endif /* ORIENTEDBOXMARKER_H */
