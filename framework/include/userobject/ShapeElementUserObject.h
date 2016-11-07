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

#ifndef SHAPEELEMENTUSEROBJECT_H
#define SHAPEELEMENTUSEROBJECT_H

#include "ElementUserObject.h"

//Forward Declarations
class ShapeElementUserObject;

template <>
InputParameters validParams<ShapeElementUserObject>();

/**
 * ElementUserObject class in which the _phi and _grad_phi shape function data
 * is available and correctly initialized on EXEC_NONLINEAR (the Jacobian calculation).
 * This enables the calculation of Jacobian matrix contributions inside a UO.
 *
 * \warning It is up to the user to ensure _fe_problem.currentlyComputingJacobian()
 *          returns true before utilizing the shape functions.
 */
class ShapeElementUserObject : public ElementUserObject
{
public:
  ShapeElementUserObject(const InputParameters & parameters);

  /// check if jacobian is to be computed in user objects
  const bool & computeJacobianFlag() const
  {
    return _compute_jacobians;
  }

  /**
   * Returns the set of variables a Jacobian has been requested for
   */
  const std::set<MooseVariable *> & jacobianMooseVariables() const
  {
    return _jacobian_moose_variables;
  }

  /**
   * This function will be called with the shape functions for jvar initialized. It
   * can be used to compute Jacobian contributions of the by implementing executeJacobian.
   */
  virtual void executeJacobianWrapper(unsigned int jvar, const std::vector<dof_id_type> & dof_indices);

protected:
  /**
   * Implement this function to compute Jacobian terms for this UserObject. The
   * shape function index _j and its corrsponding global DOF index _j_global
   * will be provided.
   */
  virtual void executeJacobian(unsigned int /*jvar*/) = 0;

  /**
   * Returns the index for a coupled variable by name and requests the computation
   * of a Jacobian w.r.t. to this variable i.e. the call to executeJacobian() with
   * shapefunctions initialized for this variable.
   */
  virtual unsigned int coupled(const std::string & var_name, unsigned int comp = 0);

  /// shape function values
  const VariablePhiValue & _phi;

  /// shape function gradients
  const VariablePhiGradient & _grad_phi;

  /// j-th index for enumerating the shape functions
  unsigned int _j;

  /// global DOF ID corresponding to _j
  dof_id_type _j_global;

private:
  const bool _compute_jacobians;
  std::set<MooseVariable *> _jacobian_moose_variables;
};

#endif //SHAPEELEMENTUSEROBJECT_H
