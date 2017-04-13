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

#ifndef ELEMENTDELETERBASE_H
#define ELEMENTDELETERBASE_H

#include "MeshModifier.h"

class ElementDeleterBase;

template <>
InputParameters validParams<ElementDeleterBase>();

/**
 * This class deletes elements from the mesh data structure
 * after it has been generated or read but before any FEM
 * data structures are initialized. Users are free to implement
 * their own derived classes by providing an implementation
 * for "shouldDelete".
 */
class ElementDeleterBase : public MeshModifier
{
public:
  ElementDeleterBase(const InputParameters & parameters);

protected:
  virtual void modify() override;

  /**
   * Method that returns a Boolean indicating whether an
   * element should be removed from the mesh.
   */
  virtual bool shouldDelete(const Elem * elem) = 0;
};

#endif /* ELEMENTDELETERBASE_H */
