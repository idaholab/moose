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

#ifndef LocalElementUserObject_H
#define LocalElementUserObject_H

// MOOSE includes
#include "ElementUserObject.h"

// Forward Declarations
class LocalElementUserObject;

template<>
InputParameters validParams<LocalElementUserObject>();

class LocalElementUserObject :
  public ElementUserObject
{
public:
  LocalElementUserObject(const InputParameters & parameters);

  /// @{ Block all methods that are not used in explicitly called UOs
  virtual void execute();
  virtual void finalize();
  virtual void threadJoin(const UserObject &);
  /// @}
};

#endif //LocalElementUserObject_H
