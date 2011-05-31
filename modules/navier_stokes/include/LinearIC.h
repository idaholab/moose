#ifndef LINEARIC_H
#define LINEARIC_H

// MOOSE Includes
#include "InitialCondition.h"

// Forward Declarations
class LinearIC;

template<>
InputParameters validParams<LinearIC>();

/**
 * LinearIC returns linear profile that goes from
 * (_xA, _valueA) to (_xB, _valueB)
 * in any single coordinate direction (0=x,1=y,2=z)
 *
 *                         x _valueB
 *
 *
 *   x _valueA
 *
 *   o.....................o
 *   _xA                   _xB
 *
 * Should probably use FunctionIC for this purpose instead.
 */
class LinearIC : public InitialCondition
{
public:

  /**
   * Constructor: Same as the rest of the MOOSE Objects
   */
  LinearIC(const std::string & name,
	   InputParameters parameters);

  /**
   * The value of the variable at a point.
   *
   * This must be overriden by derived classes.
   */
  virtual Real value(const Point & p);

private:
  // "Left" and "right" end-points
  Real _xA;
  Real _xB;

  // "Left" and "right" values
  Real _valueA;
  Real _valueB;

  // Coordinate direction (0=x,1=y,2=z)
  unsigned _coordinate_direction;
};

#endif //LINEARIC_H
