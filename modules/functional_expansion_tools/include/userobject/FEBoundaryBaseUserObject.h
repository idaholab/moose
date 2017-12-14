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

#ifndef FEBOUNDARYBASEUSEROBJECT_H
#define FEBOUNDARYBASEUSEROBJECT_H

// MOOSE includes
#include "SideIntegralVariableUserObject.h"

// Module includes
#include "FEIntegralBaseUserObject.h"

class FEBoundaryBaseUserObject;

template <>
InputParameters validParams<FEBoundaryBaseUserObject>();

class FEBoundaryBaseUserObject : public FEIntegralBaseUserObject<SideIntegralVariableUserObject>
{
public:
  /// Constructor
  FEBoundaryBaseUserObject(const InputParameters & parameters);

  /// Virtual destructor
  virtual ~FEBoundaryBaseUserObject();

protected:
  // FEIntegralBaseUserObject overrides
  virtual Point getCentroid() const final;
  virtual Real getVolume() const final;
};

#endif // FEBOUNDARYBASEUSEROBJECT_H
