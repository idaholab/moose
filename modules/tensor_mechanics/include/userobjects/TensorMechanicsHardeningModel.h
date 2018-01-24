/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSHARDENINGMODEL_H
#define TENSORMECHANICSHARDENINGMODEL_H

#include "GeneralUserObject.h"

class TensorMechanicsHardeningModel;

template <>
InputParameters validParams<TensorMechanicsHardeningModel>();

/**
 * Hardening Model base class.  The derived classes will provide
 * a value and a derivative of that value with respect to a
 * single internal parameter.
 *
 * The virtual functions written below must be
 * over-ridden in derived classes to provide actual values
 */
class TensorMechanicsHardeningModel : public GeneralUserObject
{
public:
  TensorMechanicsHardeningModel(const InputParameters & parameters);

  void initialize();
  void execute();
  void finalize();

  /* provides the value of the hardening parameter at given internal parameter
   * @param intnl the value of the internal parameter at which to evaluate the hardening parameter
   * @return the value of the hardening parameter
   */
  virtual Real value(Real intnl) const;

  /* provides d(hardening parameter)/d(internal parameter)
   * @param intnl the value of the internal parameter at which to evaluate the derivative
   * @return the value of the hardening parameter
   */
  virtual Real derivative(Real intnl) const;

  /* A name for this hardening model.  Plasticity models can use
   * this name to enable certain optimizations
   */
  virtual std::string modelName() const = 0;
};

#endif // TENSORMECHANICSHARDENINGMODEL_H
