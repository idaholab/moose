/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "FEProblem.h"

class LevelSetReinitializationProblem;

template<>
InputParameters validParams<LevelSetReinitializationProblem>();

/**
 * A Problem object to perform level set equation reinitialization implementation, mainly implementing
 * a method to reset the state of the simulation so a solve can be performed again.
 */
class LevelSetReinitializationProblem : public FEProblem
{
public:
  LevelSetReinitializationProblem(const InputParameters & parameters);

  /**
   * Resets the state of the simulation to allow for it to be re-executed.
   */
  void resetTime();
};
