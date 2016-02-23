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


template<>
InputParameters validParams<TensorMechanicsHardeningModel>();

/**
 * Hardening Model base class
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

  virtual Real value(Real intnl) const;

  virtual Real derivative(Real intnl) const;

  virtual std::string modelName() const = 0;

};

#endif // TENSORMECHANICSHARDENINGMODEL_H
