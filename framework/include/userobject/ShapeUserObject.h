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

#ifndef SHAPEUSEROBJECT_H
#define SHAPEUSEROBJECT_H

#include "MooseVariable.h"
#include "MooseObject.h"
#include "Coupleable.h"
#include "InputParameters.h"
#include "Assembly.h"

/**
 * Users of this template class must specify the type of shape functions that
 * will be used in the Jacobian calculation. Current options are for volume
 * shape functions (specified with ShapeType::Element, accessed with _assembly.phi())
 * and for surface shape functions (specified with ShapeType::Side, accessed with
 * _assembly.phiFace())
 */

enum class ShapeType
{
  Element,
  Side
};

/**
 * UserObject template class in which the _phi and _grad_phi shape function data
 * is available and correctly initialized on EXEC_NONLINEAR (the Jacobian calculation).
 * This enables the calculation of Jacobian matrix contributions inside a UO.
 *
 * \warning It is up to the user to ensure _fe_problem.currentlyComputingJacobian()
 *          returns true before utilizing the shape functions.
 */
template <typename T>
class ShapeUserObject : public T
{
public:
  ShapeUserObject(const InputParameters & parameters, ShapeType type);

  /// check if jacobian is to be computed in user objects
  const bool & computeJacobianFlag() const { return _compute_jacobians; }

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
  virtual void executeJacobianWrapper(unsigned int jvar,
                                      const std::vector<dof_id_type> & dof_indices);

  static InputParameters validParams();

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

template <typename T>
ShapeUserObject<T>::ShapeUserObject(const InputParameters & parameters, ShapeType type)
  : T(parameters),
    _phi(type == ShapeType::Element ? this->_assembly.phi() : this->_assembly.phiFace()),
    _grad_phi(type == ShapeType::Element ? this->_assembly.gradPhi()
                                         : this->_assembly.gradPhiFace()),
    _compute_jacobians(MooseObject::getParam<bool>("compute_jacobians"))
{
  mooseWarning("Jacobian calculation in UserObjects is an experimental capability with a "
               "potentially unstable interface.");
}

template <typename T>
InputParameters
ShapeUserObject<T>::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<bool>("compute_jacobians", true, "Compute Jacobians for coupled variables");
  params.addParamNamesToGroup("compute_jacobians", "Advanced");
  return params;
}

template <typename T>
unsigned int
ShapeUserObject<T>::coupled(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = Coupleable::getVar(var_name, comp);

  // add to the set of variables for which executeJacobian will be called
  if (_compute_jacobians && var->kind() == Moose::VAR_NONLINEAR)
    _jacobian_moose_variables.insert(var);

  // return the variable number
  return T::coupled(var_name, comp);
}

template <typename T>
void
ShapeUserObject<T>::executeJacobianWrapper(unsigned int jvar,
                                           const std::vector<dof_id_type> & dof_indices)
{
  for (_j = 0; _j < _phi.size(); ++_j)
  {
    _j_global = dof_indices[_j];
    executeJacobian(jvar);
  }
}

#endif // SHAPEUSEROBJECT_H
