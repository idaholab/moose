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

#ifndef BOUNDINGVALUENODALDAMPER_H
#define BOUNDINGVALUENODALDAMPER_H

// Moose Includes
#include "NodalDamper.h"

// Forward Declarations
class BoundingValueNodalDamper;

template <>
InputParameters validParams<BoundingValueNodalDamper>();

/**
 * This class implements a damper that limits the value of a variable to be within
 * user-specified bounds.
 */
class BoundingValueNodalDamper : public NodalDamper
{
public:
  BoundingValueNodalDamper(const InputParameters & parameters);

protected:
  /// The maximum permissible value of the variable
  const Real & _max_value;
  /// The minimum permissible value of the variable
  const Real & _min_value;
  /// Compute the damping for the current node
  virtual Real computeQpDamping() override;
};

#endif // BOUNDINGVALUENODALDAMPER_H
