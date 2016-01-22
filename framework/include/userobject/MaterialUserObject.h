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

#ifndef MATERIALUSEROBJECT_H
#define MATERIALUSEROBJECT_H

// MOOSE includes
#include "ElementUserObject.h"

// Forward Declarations
class MaterialUserObject;

template<>
InputParameters validParams<MaterialUserObject>();

class MaterialUserObject :
  public ElementUserObject
{
public:
  MaterialUserObject(const InputParameters & parameters);

  /// @{ Block all methods that are not used in explicitly called UOs
  virtual void execute()
  virtual void finalize()
  virtual void threadJoin(const UserObject & uo);
  /// @}
};
