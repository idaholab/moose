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

#ifndef BOUNDINGVALUEELEMENTDAMPER_H
#define BOUNDINGVALUEELEMENTDAMPER_H

// Moose Includes
#include "ElementDamper.h"

// Forward Declarations
class BoundingValueElementDamper;

template <>
InputParameters validParams<BoundingValueElementDamper>();

/**
 * This class implements a damper that limits the value of a variable to be within
 * user-specified bounds.
 */
class BoundingValueElementDamper : public ElementDamper
{
public:
  BoundingValueElementDamper(const InputParameters & parameters);

protected:
  /// The maximum permissible value of the variable
  const Real & _max_value;
  /// The minimum permissible value of the variable
  const Real & _min_value;
  /// Compute the damping for the current qp
  virtual Real computeQpDamping() override;
};

#endif // BOUNDINGVALUEELEMENTDAMPER_H
