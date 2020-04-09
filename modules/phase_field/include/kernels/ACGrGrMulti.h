//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ACGrGrBase.h"

// Forward Declarations

/**
 * This kernel calculates the residual for grain growth for a multi-phase,
 * poly-crystal system. A list of material properties needs to be supplied for the gammas
 * (prefactors of the cross-terms between order parameters).
 */
class ACGrGrMulti : public ACGrGrBase
{
public:
  static InputParameters validParams();

  ACGrGrMulti(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Names of gammas for each order parameter
  std::vector<MaterialPropertyName> _gamma_names;
  unsigned int _num_j;

  /// Values of gammas for each order parameter
  std::vector<const MaterialProperty<Real> *> _prop_gammas;

  const NonlinearVariableName _uname;
  const MaterialProperty<Real> & _dmudu;
  const std::vector<VariableName> _vname;
  std::vector<const MaterialProperty<Real> *> _dmudEtaj;

private:
  Real computedF0du();
};
