#ifndef TEJUMPBC_H
#define TEJUMPBC_H

#include "BoundaryCondition.h"

class TEJumpBC;
class Function;

template<>
InputParameters validParams<TEJumpBC>();

/**
 * Implements a BC for TimeError test case
 *
 */

class TEJumpBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same constructor.
   */
  TEJumpBC( const std::string & name, InputParameters parameters);

  virtual ~TEJumpBC() {}

protected:
  virtual Real computeQpResidual();

  Real _t_jump;
  Real _slope;
};

#endif
