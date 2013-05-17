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

// System includes
#include <string>

class Problem;
class MeshModifier;

template<>
InputParameters validParams<MeshModifier>();

/**
 * MeshModifiers are objects that can modify or add to an existing mesh.
 */
class MeshModifier : public MooseObject
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the MeshModifier in the input file.
   * @param parameters The parameters object holding data for the class to use.
   */
  MeshModifier(const std::string & name, InputParameters parameters);

  virtual ~MeshModifier();

  /**
   * This function gets called prior to modify to set the current
   * mesh pointer.
   */
  void setMeshPointer(MooseMesh *mesh) { _mesh_ptr = mesh; }

  /**
   * Pure virtual modify function MUST be overriden by children classes.
   * This is where the MeshModifier actually does it's work.
   */
  virtual void modify() = 0;

protected:
  MooseMesh *_mesh_ptr;
};

#endif //MESHMODIFIER_H
