/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COUPLEDCOEFREACTION_H
#define COUPLEDCOEFREACTION_H

#include "CoupledReaction.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declaration
class CoupledCoefReaction;

template<>
InputParameters validParams<CoupledCoefReaction>();

// This kernel adds to the residual a contribution of -L*v where L is a material
// property and v is a coupled variable
class CoupledCoefReaction : public DerivativeMaterialInterface<JvarMapInterface<CoupledReaction> >
{
public:
  CoupledCoefReaction(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Coupled variable (For constrained Allen-Cahn problems, v = lambda
  /// where lambda is the Lagrange multiplier)
  std::string _v_name;
  VariableValue & _v;
  unsigned int _v_var;

  /// Mobility
  const MaterialProperty<Real> & _L;

  /// name of the order parameter (needed to retrieve the derivative material properties)
  VariableName _eta_name;

  ///  Mobility derivative w.r.t. order parameter
  const MaterialProperty<Real> & _dLdop;

  ///  Mobility derivative w.r.t. the coupled variable being added by this kernel
  const MaterialProperty<Real> & _dLdv;

  /// number of coupled variables
  const unsigned int _nvar;

  ///  Mobility derivatives w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dLdarg;

};

#endif //COUPLEDCOEFREACTION_H
