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
#include "Restartable.h"

// Forward declarations
class MeshModifier;
class MooseMesh;

template<>
InputParameters validParams<MeshModifier>();

/**
 * MeshModifiers are objects that can modify or add to an existing mesh.
 */
class MeshModifier :
  public MooseObject,
  public Restartable
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  MeshModifier(const InputParameters & parameters);

  virtual ~MeshModifier();

  /**
   * This function gets called prior to modify to set the current
   * mesh pointer.
   */
  void setMeshPointer(MooseMesh *mesh) { _mesh_ptr = mesh; }

  /**
   * Pure virtual modify function MUST be overridden by children classes.
   * This is where the MeshModifier actually does it's work.
   */
  virtual void modify() = 0;

  std::vector<std::string> & getDependencies() { return _depends_on; }

  bool forcePrepare() const { return _force_prepare; }

protected:
  MooseMesh *_mesh_ptr;

private:
  /// A list of modifiers that are required to run before this modifier may run
  std::vector<std::string> _depends_on;

  /// Flag to determine if the mesh should be prepared after this modifier is run
  const bool _force_prepare;
};

#endif //MESHMODIFIER_H
