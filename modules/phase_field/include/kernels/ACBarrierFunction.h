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

/**
 * Several kernels use a material property called mu. If mu is not a constant,
 * then this kernel will calculate the bulk AC term where mu is the derivative term.
 * It currently only takes a single value for gamma.
 **/
class ACBarrierFunction : public ACGrGrBase
{
public:
  static InputParameters validParams();

  ACBarrierFunction(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _n_eta;
  const NonlinearVariableName _uname;
  const MaterialPropertyName _gamma_name;
  const MaterialProperty<Real> & _gamma;
  const MaterialProperty<Real> & _dmudvar;
  const MaterialProperty<Real> & _d2mudvar2;

  const std::vector<VariableName> _vname;
  std::vector<const MaterialProperty<Real> *> _d2mudvardeta;
  const JvarMap & _vmap;

private:
  Real calculateF0(); /// calculates the free energy function
};
