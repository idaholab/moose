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

#ifndef DISCRETEELEMENTUSEROBJECT_H
#define DISCRETEELEMENTUSEROBJECT_H

// MOOSE includes
#include "ElementUserObject.h"

// Forward Declarations
class DiscreteElementUserObject;

template<>
InputParameters validParams<DiscreteElementUserObject>();

class DiscreteElementUserObject :
  public ElementUserObject
{
public:
  DiscreteElementUserObject(const InputParameters & parameters);

  virtual void initialize();

  /// @{ Block all methods that are not used in explicitly called UOs
  virtual void execute(); // libmesh_final;
  virtual void finalize(); // libmesh_final;
  virtual void threadJoin(const UserObject &); // libmesh_final;
  /// @}
};

#endif //DISCRETEELEMENTUSEROBJECT_H
