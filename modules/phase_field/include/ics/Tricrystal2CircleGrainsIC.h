#ifndef TRICRYSTAL2CIRCLEGRAINSIC_H
#define TRICRYSTAL2CIRCLEGRAINSIC_H

#include "Kernel.h"
#include "InitialCondition.h"

// System includes
#include <string>

// Forward Declarations
class Tricrystal2CircleGrainsIC;

template<>
InputParameters validParams<Tricrystal2CircleGrainsIC>();

/**
 * Tricrystal2CircleGrainsIC creates a 3 grain structure with 2 circle grains and one matrix grain
*/
class Tricrystal2CircleGrainsIC : public InitialCondition
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  Tricrystal2CircleGrainsIC(const std::string & name,
                InputParameters parameters);

  /**
   * The value of the variable at a point.
   *
   * This must be overriden by derived classes.
   */
  virtual Real value(const Point & p);

protected:
  MooseMesh & _mesh;
  /// A reference to the nonlinear system
  NonlinearSystem & _nl;

  unsigned int _op_num;
  unsigned int _op_index;

  Point _bottom_left;
  Point _top_right;
  Point _range;
};

#endif //TRICRYSTAL2CIRCLEGRAINSIC_H
