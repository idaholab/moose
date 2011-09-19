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

#ifndef MESHMODIFIER_H
#define MESHMODIFIER_H

#include "MooseObject.h"
#include "MooseMesh.h"

class MeshModifier;

template<>
InputParameters validParams<MeshModifier>();


class MeshModifier : public MooseObject
{
public:
  MeshModifier(const std::string & name, InputParameters parameters);

  virtual void modifyMesh(Mesh & mesh) = 0;

};

#endif /* MESHMODIFIER_H */
