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

#ifndef BLOCKDELETER_H
#define BLOCKDELETER_H

#include "MeshModifier.h"

class ElementDeleterBase;

template<>
InputParameters validParams<ElementDeleterBase>();

class ElementDeleterBase : public MeshModifier
{
public:
  ElementDeleterBase(const InputParameters & parameters);

  virtual void modify();

  // TODO: This should be made pure virtual and overridden by child classes
  virtual bool shouldDelete(const Elem * elem);
};

#endif /* BLOCKDELETER_H */
