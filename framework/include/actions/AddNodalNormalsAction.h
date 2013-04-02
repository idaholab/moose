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

#ifndef ADDNODALNORMALSACTION_H
#define ADDNODALNORMALSACTION_H

#include "Action.h"


class AddNodalNormalsAction;

template<>
InputParameters validParams<AddNodalNormalsAction>();

/**
 * Action to setup computation of nodal normals.
 *
 * The machinery behind the normal computation is the following:
 * - NodalNormalsPreprocessor is ran over the elements and gather the \int \d grad_phi \over \d {x|y|z} \d Omega into three separate vectors
 *   (that live on AuxiliarySystem) - each for one component of the normal.
 * - NodalNormalsEvaluator is than ran over the boundary nodes and takes the above computed integrals and normalizes it.
 *
 * NOTE: the auxiliary system has to have at least one variable on it, so that the vectors for nx, ny and nz have non-zero length.
 */
class AddNodalNormalsAction : public Action
{
public:
  AddNodalNormalsAction(const std::string & name, InputParameters parameters);
  virtual ~AddNodalNormalsAction();

  virtual void act();

protected:
  std::vector<BoundaryName> _boundary;
};

#endif /* ADDNODALNORMALSACTION_H */
