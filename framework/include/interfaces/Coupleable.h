//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEABLE_H
#define COUPLEABLE_H

#include <map>
#include "MooseTypes.h"
#include "MooseArray.h"
#include "MooseVariableFE.h"
#include "InputParameters.h"

// Forward declarations
class MooseVariableScalar;
class MooseObject;

namespace libMesh
{
template <typename T>
class DenseVector;
}

#define adCoupledValue this->template adCoupledValueTemplate<compute_stage>
#define adCoupledGradient this->template adCoupledGradientTemplate<compute_stage>
#define adZero this->template adZeroTemplate<compute_stage>
#define adGradZero this->template adGradZeroTemplate<compute_stage>

/**
 * Interface for objects that needs coupling capabilities
 *
 */
class Coupleable
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   * @param nodal true if we need to couple with nodal values, otherwise false
   */
  Coupleable(const MooseObject * moose_object, bool nodal);

  /**
   * Destructor for object
   */
  virtual ~Coupleable();

  /**
   * Get the list of coupled variables
   * @return The list of coupled variables
   */
  const std::map<std::string, std::vector<MooseVariableFEBase *>> & getCoupledVars()
  {
    return _coupled_vars;
  }

  /**
   * Get the list of all coupled variables
   * @return The list of all coupled variables
   */
  const std::vector<MooseVariableFEBase *> & getCoupledMooseVars() const
  {
    return _coupled_moose_vars;
  }

  /**
   * Get the list of standard coupled variables
   * @return The list of standard coupled variables
   */
  const std::vector<MooseVariable *> & getCoupledStandardMooseVars() const
  {
    return _coupled_standard_moose_vars;
  }

  /**
   * Get the list of vector coupled variables
   * @return The list of vector coupled variables
   */
  const std::vector<VectorMooseVariable *> & getCoupledVectorMooseVars() const
  {
    return _coupled_vector_moose_vars;
  }

  void addFEVariableCoupleableVectorTag(TagID tag) { _fe_coupleable_vector_tags.insert(tag); }

  void addFEVariableCoupleableMatrixTag(TagID tag) { _fe_coupleable_matrix_tags.insert(tag); }

  std::set<TagID> & getFEVariableCoupleableVectorTags() { return _fe_coupleable_vector_tags; }

  std::set<TagID> & getFEVariableCoupleableMatrixTags() { return _fe_coupleable_matrix_tags; }

protected:
  /**
   * Returns true if a variables has been coupled as name.
   * @param var_name The name the kernel wants to refer to the variable as.
   * @param i By default 0, in general the index to test in a vector of MooseVariable pointers.
   * @return True if a coupled variable has the supplied name
   */
  virtual bool isCoupled(const std::string & var_name, unsigned int i = 0);

  /**
   * Number of coupled components
   * @param var_name Name of the variable
   * @return number of components this variable has (usually 1)
   */
  unsigned int coupledComponents(const std::string & var_name);

  virtual void coupledCallback(const std::string & var_name, bool is_old);

  /**
   * Returns the index for a coupled variable by name
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Index of coupled variable, if this is an optionally coupled variable that wasn't
   * provided this will return a unique "invalid" index.
   */
  virtual unsigned int coupled(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns value of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::_u
   */
  virtual const VariableValue & coupledValue(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns value of a coupled variable for use in Automatic Differentiation
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::value
   */
  template <ComputeStage compute_stage>
  const typename VariableValueType<compute_stage>::type &
  adCoupledValueTemplate(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns value of a coupled variable for a given tag
   * @param var_name Name of coupled variable
   * @param tag vector tag ID
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::_u
   */
  virtual const VariableValue &
  coupledVectorTagValue(const std::string & var_name, TagID tag, unsigned int comp = 0);

  /**
   * Returns value of a coupled variable for a given tag. This couples the diag vector of matrix
   * @param var_name Name of coupled variable
   * @param tag matrix tag ID
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::_u
   */
  virtual const VariableValue &
  coupledMatrixTagValue(const std::string & var_name, TagID tag, unsigned int comp = 0);

  /**
   * Returns value of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue for the coupled vector variable
   * @see VectorKernel::_u
   */
  virtual const VectorVariableValue & coupledVectorValue(const std::string & var_name,
                                                         unsigned int comp = 0);

  /**
   * Returns a *writable* reference to a coupled variable.  Note: you
   * should not have to use this very often (use coupledValue()
   * instead) but there are situations, such as writing to multiple
   * AuxVariables from a single AuxKernel, where it is required.
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::value
   */
  virtual VariableValue & writableCoupledValue(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns an old value from previous time step  of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the old value of the coupled variable
   * @see Kernel::valueOld
   */
  virtual const VariableValue & coupledValueOld(const std::string & var_name,
                                                unsigned int comp = 0);

  /**
   * Returns an old value from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the older value of the coupled variable
   * @see Kernel::valueOlder
   */
  virtual const VariableValue & coupledValueOlder(const std::string & var_name,
                                                  unsigned int comp = 0);

  /**
   * Returns value of previous Newton iterate of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the older value of the coupled variable
   */
  virtual const VariableValue & coupledValuePreviousNL(const std::string & var_name,
                                                       unsigned int comp = 0);

  /**
   * Returns an old value from previous time step  of a coupled vector variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableValue containing the old value of the coupled variable
   * @see Kernel::_u_old
   */
  virtual const VectorVariableValue & coupledVectorValueOld(const std::string & var_name,
                                                            unsigned int comp = 0);

  /**
   * Returns an old value from two time steps previous of a coupled vector variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableValue containing the older value of the coupled variable
   * @see Kernel::_u_older
   */
  virtual const VectorVariableValue & coupledVectorValueOlder(const std::string & var_name,
                                                              unsigned int comp = 0);

  /**
   * Returns gradient of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the gradient of the coupled variable
   * @see Kernel::gradient
   */
  virtual const VariableGradient & coupledGradient(const std::string & var_name,
                                                   unsigned int comp = 0);

  /**
   * Returns gradient of a coupled variable for use in Automatic Differentation
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the gradient of the coupled variable
   * @see Kernel::gradient
   */
  template <ComputeStage compute_stage>
  const typename VariableGradientType<compute_stage>::type &
  adCoupledGradientTemplate(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns an old gradient from previous time step of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the old gradient of the coupled variable
   * @see Kernel::gradientOld
   */
  virtual const VariableGradient & coupledGradientOld(const std::string & var_name,
                                                      unsigned int comp = 0);

  /**
   * Returns an old gradient from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the older gradient of the coupled variable
   * @see Kernel::gradientOlder
   */
  virtual const VariableGradient & coupledGradientOlder(const std::string & var_name,
                                                        unsigned int comp = 0);

  /**
   * Returns gradient of a coupled variable for previous Newton iterate
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the gradient of the coupled variable
   */
  virtual const VariableGradient & coupledGradientPreviousNL(const std::string & var_name,
                                                             unsigned int comp = 0);

  /**
   * Time derivative of the gradient of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the time derivative of the gradient of a
   * coupled variable
   */
  virtual const VariableGradient & coupledGradientDot(const std::string & var_name,
                                                      unsigned int comp = 0);

  /**
   * Second time derivative of the gradient of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the time derivative of the gradient of a
   * coupled variable
   */
  virtual const VariableGradient & coupledGradientDotDot(const std::string & var_name,
                                                         unsigned int comp = 0);

  /**
   * Returns gradient of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableGradient containing the gradient of the coupled vector
   * variable
   */
  virtual const VectorVariableGradient & coupledVectorGradient(const std::string & var_name,
                                                               unsigned int comp = 0);

  /**
   * Returns an old gradient from previous time step of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableGradient containing the old gradient of the coupled vector
   * variable
   */
  virtual const VectorVariableGradient & coupledVectorGradientOld(const std::string & var_name,
                                                                  unsigned int comp = 0);

  /**
   * Returns an old gradient from two time steps previous of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableGradient containing the older gradient of the coupled
   * vector variable
   */
  virtual const VectorVariableGradient & coupledVectorGradientOlder(const std::string & var_name,
                                                                    unsigned int comp = 0);

  /**
   * Returns curl of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableCurl containing the curl of the coupled variable
   * @see Kernel::_curl_u
   */
  virtual const VectorVariableCurl & coupledCurl(const std::string & var_name,
                                                 unsigned int comp = 0);

  /**
   * Returns an old curl from previous time step of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableCurl containing the old curl of the coupled variable
   * @see Kernel::_curl_u_old
   */
  virtual const VectorVariableCurl & coupledCurlOld(const std::string & var_name,
                                                    unsigned int comp = 0);

  /**
   * Returns an old curl from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VectorVariableCurl containing the older curl of the coupled variable
   * @see Kernel::_curl_u_older
   */
  virtual const VectorVariableCurl & coupledCurlOlder(const std::string & var_name,
                                                      unsigned int comp = 0);

  /**
   * Returns second derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the second derivative of the coupled variable
   * @see Kernel::second
   */
  virtual const VariableSecond & coupledSecond(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns an old second derivative from previous time step of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the old second derivative of the coupled
   * variable
   * @see Kernel::secondOld
   */
  virtual const VariableSecond & coupledSecondOld(const std::string & var_name,
                                                  unsigned int comp = 0);

  /**
   * Returns an old second derivative from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the older second derivative of the coupled
   * variable
   * @see Kernel::secondOlder
   */
  virtual const VariableSecond & coupledSecondOlder(const std::string & var_name,
                                                    unsigned int comp = 0);

  /**
   * Returns second derivative of a coupled variable for the previous Newton iterate
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the second derivative of the coupled variable
   */
  virtual const VariableSecond & coupledSecondPreviousNL(const std::string & var_name,
                                                         unsigned int comp = 0);

  /**
   * Time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   */
  virtual const VariableValue & coupledDot(const std::string & var_name, unsigned int comp = 0);

  /**
   * Second time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   */
  virtual const VariableValue & coupledDotDot(const std::string & var_name, unsigned int comp = 0);

  /**
   * Old time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   */
  virtual const VariableValue & coupledDotOld(const std::string & var_name, unsigned int comp = 0);

  /**
   * Old second time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   */
  virtual const VariableValue & coupledDotDotOld(const std::string & var_name,
                                                 unsigned int comp = 0);

  /**
   * Time derivative of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const VectorVariableValue & coupledVectorDot(const std::string & var_name,
                                                       unsigned int comp = 0);

  /**
   * Second time derivative of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const VectorVariableValue & coupledVectorDotDot(const std::string & var_name,
                                                          unsigned int comp = 0);

  /**
   * Old time derivative of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const VectorVariableValue & coupledVectorDotOld(const std::string & var_name,
                                                          unsigned int comp = 0);

  /**
   * Old second time derivative of a coupled vector variable
   * @param var_name Name of coupled vector variable
   * @param comp Component number for vector of coupled vector variables
   * @return Reference to a VectorVariableValue containing the time derivative of the coupled
   * variable
   */
  virtual const VectorVariableValue & coupledVectorDotDotOld(const std::string & var_name,
                                                             unsigned int comp = 0);

  /**
   * Time derivative of a coupled variable with respect to the coefficients
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   * with respect to the coefficients
   */
  virtual const VariableValue & coupledDotDu(const std::string & var_name, unsigned int comp = 0);

  /**
   * Second time derivative of a coupled variable with respect to the coefficients
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   * with respect to the coefficients
   */
  virtual const VariableValue & coupledDotDotDu(const std::string & var_name,
                                                unsigned int comp = 0);

  /**
   * Returns nodal values of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   */
  virtual const VariableValue & coupledNodalValue(const std::string & var_name,
                                                  unsigned int comp = 0);

  /**
   * Returns an old nodal value from previous time step  of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the old value of the coupled variable
   */
  virtual const VariableValue & coupledNodalValueOld(const std::string & var_name,
                                                     unsigned int comp = 0);

  /**
   * Returns an old nodal value from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the older value of the coupled variable
   */
  virtual const VariableValue & coupledNodalValueOlder(const std::string & var_name,
                                                       unsigned int comp = 0);

  /**
   * Returns nodal values of a coupled variable for previous Newton iterate
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   */
  virtual const VariableValue & coupledNodalValuePreviousNL(const std::string & var_name,
                                                            unsigned int comp = 0);

  /**
   * Nodal values of time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the nodal values of time derivative of the
   * coupled variable
   */
  virtual const VariableValue & coupledNodalDot(const std::string & var_name,
                                                unsigned int comp = 0);

  /**
   * Nodal values of second time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the nodal values of second time derivative of
   * the coupled variable
   */
  virtual const VariableValue & coupledNodalDotDot(const std::string & var_name,
                                                   unsigned int comp = 0);

  /**
   * Nodal values of old time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the nodal values of time derivative of the
   * coupled variable
   */
  virtual const VariableValue & coupledNodalDotOld(const std::string & var_name,
                                                   unsigned int comp = 0);

  /**
   * Nodal values of old second time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the nodal values of second time derivative of
   * the coupled variable
   */
  virtual const VariableValue & coupledNodalDotDotOld(const std::string & var_name,
                                                      unsigned int comp = 0);
  /**
   * Returns DoFs in the current solution vector of a coupled variable for the local element
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a DenseVector for the DoFs of the coupled variable
   */
  virtual const DenseVector<Number> & coupledSolutionDoFs(const std::string & var_name,
                                                          unsigned int comp = 0);

  /**
   * Returns DoFs in the old solution vector of a coupled variable for the local element
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a DenseVector for the old DoFs of the coupled variable
   */
  virtual const DenseVector<Number> & coupledSolutionDoFsOld(const std::string & var_name,
                                                             unsigned int comp = 0);

  /**
   * Returns DoFs in the older solution vector of a coupled variable for the local element
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a DenseVector for the older DoFs of the coupled variable
   */
  virtual const DenseVector<Number> & coupledSolutionDoFsOlder(const std::string & var_name,
                                                               unsigned int comp = 0);

  /**
   * Template method that returns _zero to RESIDUAL computing objects and _ad_zero to JACOBIAN
   * computing objects
   */
  template <ComputeStage compute_stage>
  const typename VariableValueType<compute_stage>::type & adZeroTemplate();

  /**
   * Template method that returns _grad_zero to RESIDUAL computing objects and _ad_grad_zero to
   * JACOBIAN computing objects
   */
  template <ComputeStage compute_stage>
  const typename VariableGradientType<compute_stage>::type & adGradZeroTemplate();

protected:
  // Reference to the interface's input parameters
  const InputParameters & _c_parameters;

  /// The name of the object this interface is part of
  const std::string & _c_name;

  // Reference to FEProblemBase
  FEProblemBase & _c_fe_problem;

  /// Coupled vars whose values we provide
  std::map<std::string, std::vector<MooseVariableFEBase *>> _coupled_vars;

  /// Vector of all coupled variables
  std::vector<MooseVariableFEBase *> _coupled_moose_vars;

  /// Vector of standard coupled variables
  std::vector<MooseVariable *> _coupled_standard_moose_vars;

  /// Vector of vector coupled variables
  std::vector<VectorMooseVariable *> _coupled_vector_moose_vars;

  /// True if we provide coupling to nodal values
  bool _c_nodal;

  /// True if implicit value is required
  bool _c_is_implicit;

  /// Thread ID of the thread using this object
  THREAD_ID _c_tid;

  /// Will hold the default value for optional coupled variables.
  std::map<std::string, std::vector<VariableValue *>> _default_value;

  /// Will hold the default value for optional coupled variables for automatic differentiation.
  std::map<std::string, MooseArray<ADReal> *> _ad_default_value;

  /// Will hold the default value for optional vector coupled variables.
  std::map<std::string, VectorVariableValue *> _default_vector_value;

  /**
   * This will always be zero because the default values for optionally coupled variables is always
   * constant and this is used for time derivative info
   */
  VariableValue _default_value_zero;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  VariableGradient _default_gradient;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  MooseArray<ADRealGradient> _ad_default_gradient;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  VariableSecond _default_second;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  MooseArray<ADRealTensor> _ad_default_second;

  /// Zero value of a variable
  const VariableValue & _zero;
  const MooseArray<ADReal> & _ad_zero;

  /// Zero gradient of a variable
  const VariableGradient & _grad_zero;
  const MooseArray<ADRealGradient> & _ad_grad_zero;

  /// Zero second derivative of a variable
  const VariableSecond & _second_zero;
  /// Zero second derivative of a test function
  const VariablePhiSecond & _second_phi_zero;
  /// Zero value of a vector variable
  const VectorVariableValue & _vector_zero;
  /// Zero value of the curl of a vector variable
  const VectorVariableCurl & _vector_curl_zero;

  /**
   * This will always be zero because the default values for optionally coupled variables is always
   * constant and this is used for time derivative info
   */
  VectorVariableValue _default_vector_value_zero;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  VectorVariableGradient _default_vector_gradient;

  /// This will always be zero because the default values for optionally coupled variables is always constant
  VectorVariableCurl _default_vector_curl;

  /**
   * Check that the right kind of variable is being coupled in
   *
   * @param var_name The name of the coupled variable
   */
  void checkVar(const std::string & var_name);

  /**
   * Extract pointer to a base finite element coupled variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  MooseVariableFEBase * getFEVar(const std::string & var_name, unsigned int comp);

  /**
   * Extract pointer to a coupled variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  MooseVariable * getVar(const std::string & var_name, unsigned int comp);

  /**
   * Extract pointer to a coupled vector variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  VectorMooseVariable * getVectorVar(const std::string & var_name, unsigned int comp);

  /**
   * Checks to make sure that the current Executioner has set "_is_transient" when old/older values
   * are coupled in.
   * @param name the name of the variable
   * @param fn_name The name of the function that called this method - used in the error message
   */
  void validateExecutionerType(const std::string & name, const std::string & fn_name) const;

  /// Whether or not this object is a "neighbor" object: ie all of it's coupled values should be neighbor values
  bool _coupleable_neighbor;

private:
  /**
   * Helper method to return (and insert if necessary) the default value
   * for an uncoupled variable.
   * @param var_name the name of the variable for which to retrieve a default value
   * @return a pointer to the associated VariableValue.
   */
  VariableValue * getDefaultValue(const std::string & var_name, unsigned int comp);

  /**
   * Helper method to return (and insert if necessary) the default value for Automatic
   * Differentiation for an uncoupled variable.
   * @param var_name the name of the variable for which to retrieve a default value
   * @return VariableValue * a pointer to the associated VarirableValue.
   */
  template <ComputeStage compare_stage>
  typename VariableValueType<compare_stage>::type * getADDefaultValue(const std::string & var_name);

  /**
   * Helper method to return (and insert if necessary) the default gradient for Automatic
   * Differentiation for an uncoupled variable.
   * @param var_name the name of the variable for which to retrieve a default gradient
   * @return VariableGradient * a pointer to the associated VariableGradient.
   */
  template <ComputeStage compare_stage>
  typename VariableGradientType<compare_stage>::type & getADDefaultGradient();

  /**
   * Helper method to return (and insert if necessary) the default value
   * for an uncoupled vector variable.
   * @param var_name the name of the vector variable for which to retrieve a default value
   * @return a pointer to the associated VectorVariableValue.
   */
  VectorVariableValue * getVectorDefaultValue(const std::string & var_name);

  /// Maximum qps for any element in this system
  unsigned int _coupleable_max_qps;

  /// Unique indices for optionally coupled vars that weren't provided
  std::map<std::string, std::vector<unsigned int>> _optional_var_index;

  /// Scalar variables coupled into this object (for error checking)
  std::map<std::string, std::vector<MooseVariableScalar *>> _c_coupled_scalar_vars;

  std::set<TagID> _fe_coupleable_vector_tags;

  std::set<TagID> _fe_coupleable_matrix_tags;
};

template <ComputeStage compute_stage>
const typename VariableValueType<compute_stage>::type &
Coupleable::adCoupledValueTemplate(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name))
    return *getADDefaultValue<compute_stage>(var_name);

  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      mooseError("Not implemented");
    else
    {
      if (_c_is_implicit)
        return var->adSln<compute_stage>();
      else
        mooseError("Not implemented");
    }
  }
  else
  {
    if (_c_nodal)
      mooseError("Not implemented");
    else
    {
      if (_c_is_implicit)
        return var->adSlnNeighbor<compute_stage>();
      else
        mooseError("Not implemented");
    }
  }
}

template <ComputeStage compute_stage>
const typename VariableGradientType<compute_stage>::type &
Coupleable::adCoupledGradientTemplate(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return getADDefaultGradient<compute_stage>();

  coupledCallback(var_name, false);
  if (_c_nodal)
    mooseError("Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);

  if (!_coupleable_neighbor)
  {
    if (_c_is_implicit)
      return var->adGradSln<compute_stage>();
    else
      mooseError("Not implemented");
  }
  else
  {
    if (_c_is_implicit)
      return var->adGradSlnNeighbor<compute_stage>();
    else
      mooseError("Not implemented");
  }
}

template <ComputeStage compute_stage>
typename VariableValueType<compute_stage>::type *
Coupleable::getADDefaultValue(const std::string & var_name)
{
  std::map<std::string, MooseArray<ADReal> *>::iterator default_value_it =
      _ad_default_value.find(var_name);
  if (default_value_it == _ad_default_value.end())
  {
    ADVariableValue * value =
        new ADVariableValue(_coupleable_max_qps, _c_parameters.defaultCoupledValue(var_name));
    default_value_it = _ad_default_value.insert(std::make_pair(var_name, value)).first;
  }

  return default_value_it->second;
}

template <>
VariableValue * Coupleable::getADDefaultValue<RESIDUAL>(const std::string & var_name);

template <ComputeStage compute_stage>
typename VariableGradientType<compute_stage>::type &
Coupleable::getADDefaultGradient()
{
  return _ad_default_gradient;
}

template <>
VariableGradient & Coupleable::getADDefaultGradient<RESIDUAL>();

template <ComputeStage compute_stage>
const typename VariableValueType<compute_stage>::type &
Coupleable::adZeroTemplate()
{
  return _zero;
}

template <>
const typename VariableValueType<JACOBIAN>::type & Coupleable::adZeroTemplate<JACOBIAN>();

template <ComputeStage compute_stage>
const typename VariableGradientType<compute_stage>::type &
Coupleable::adGradZeroTemplate()
{
  return _grad_zero;
}

template <>
const typename VariableGradientType<JACOBIAN>::type & Coupleable::adGradZeroTemplate<JACOBIAN>();

#endif /* COUPLEABLE_H */
