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

#ifndef MATERIALDERIVATIVETESTACTION_H
#define MATERIALDERIVATIVETESTACTION_H

#include "Action.h"
#include "DerivativeMaterialPropertyNameInterface.h"

class MaterialDerivativeTestAction;

template <>
InputParameters validParams<MaterialDerivativeTestAction>();

/**
 * Sets up variables and Kernels to test the derivatives of material properties via
 * the Jacobian checker
 */
class MaterialDerivativeTestAction : public Action, public DerivativeMaterialPropertyNameInterface
{
public:
  MaterialDerivativeTestAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  std::vector<VariableName> _args;

  MaterialPropertyName _prop_name;

  enum class PropTypeEnum
  {
    REAL,
    RANKTWOTENSOR,
    RANKFOURTENSOR
  } _prop_type;

  const unsigned int _derivative_order;

  const bool _second;

  /// every derivative given by a list of variables to derive w.r.t
  std::map<MaterialPropertyName, std::vector<VariableName>> _derivatives;
};

#endif // MATERIALDERIVATIVETESTACTION_H
