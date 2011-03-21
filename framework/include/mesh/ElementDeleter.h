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

#ifndef ELEMENTDELETER_H
#define ELEMENTDELETER_H

#include "MeshModifier.h"
#include "Function.h"
#include "FunctionInterface.h"

class ElementDeleter : public MeshModifier
// TODO: Make this work
//                     , FunctionInterface
{
public:
  ElementDeleter(const std::string & name, InputParameters parameters);

  virtual void modifyMesh(Mesh & mesh);

private:
  void removeAllElemBCs(Mesh & mesh, Elem * elem);
};

#endif /* ELEMENTDELETER_H */
