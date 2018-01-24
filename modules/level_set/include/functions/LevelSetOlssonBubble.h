//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LEVELSETOLSSONBUBBLE_H
#define LEVELSETOLSSONBUBBLE_H

// MOOSE includes
#include "Function.h"

class LevelSetOlssonBubble;

template <>
InputParameters validParams<LevelSetOlssonBubble>();

/**
 * Implements the "bubble" function from Olsson and Kreiss (2005).
 */
class LevelSetOlssonBubble : public Function
{
public:
  LevelSetOlssonBubble(const InputParameters & parameters);

  virtual Real value(Real /*t*/, const Point & p) override;

  virtual RealGradient gradient(Real /*t*/, const Point & p) override;

protected:
  /// The 'center' of the bubble
  const RealVectorValue & _center;

  /// The radius of the bubble
  const Real & _radius;

  /// The interface thickness
  const Real & _epsilon;
};

#endif // LEVELSETOLSSONBUBBLE_H
