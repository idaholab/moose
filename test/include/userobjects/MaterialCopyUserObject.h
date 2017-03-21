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

#ifndef MATERIALCOPYUSEROBJECT_H
#define MATERIALCOPYUSEROBJECT_H

#include "GeneralUserObject.h"

// Forward Declarations
class MaterialCopyUserObject;

template <>
InputParameters validParams<MaterialCopyUserObject>();

class MaterialCopyUserObject : public GeneralUserObject
{
public:
  MaterialCopyUserObject(const InputParameters & parameters);

  virtual ~MaterialCopyUserObject() {}

  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize() {}

  /**
   * Compute the hit positions for this timestep
   */
  virtual void execute();

  virtual void finalize() {}

protected:
  MooseMesh & _mesh;
  std::vector<Real> _copy_times;
  unsigned int _copy_from_element;
  unsigned int _copy_to_element;
  Real _time_tol;
};

#endif
