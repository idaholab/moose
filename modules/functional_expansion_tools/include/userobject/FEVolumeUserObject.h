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

#ifndef FEVOLUMEUSEROBJECT_H
#define FEVOLUMEUSEROBJECT_H

// MOOSE includes
#include "ElementIntegralVariableUserObject.h"
#include "FEIntegralBaseUserObject.h"

// Forward declarations
class FEVolumeUserObject;

template <>
InputParameters validParams<FEVolumeUserObject>();

/// This volume variant depends on ElementIntegralVariableUserObject
class FEVolumeUserObject final : public FEIntegralBaseUserObject<ElementIntegralVariableUserObject>
{
public:
  /// Constructor
  FEVolumeUserObject(const InputParameters & parameters);

  /// Virtual destructor
  virtual ~FEVolumeUserObject();

protected:
  // FEIntegralBaseUserObject overrides
  virtual Point getCentroid() const;
  virtual Real getVolume() const;
};

#endif // FEVOLUMEUSEROBJECT_H
