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

#ifndef FEBOUNDARYVALUEUSEROBJECT_H
#define FEBOUNDARYVALUEUSEROBJECT_H

// Module includes
#include "FEBoundaryBaseUserObject.h"

// Forward declarations
class FEBoundaryValueUserObject;

template <>
InputParameters validParams<FEBoundaryValueUserObject>();

/// This boundary variant depends on SideIntegralVariableUserObject
class FEBoundaryValueUserObject final : public FEBoundaryBaseUserObject
{
public:
  /// Constructor
  FEBoundaryValueUserObject(const InputParameters & parameters);

  /// Virtual destructor
  virtual ~FEBoundaryValueUserObject();
};

#endif // FEBOUNDARYVALUEUSEROBJECT_H
