//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 *  Computes the time derivative of the azimuthal component
 *  of the magnetic field assuming cylindrical electric field. The electric field is
 *  is supplied as a vector.
 */
class AzimuthMagneticTimeDerivVectorRZ : public AuxKernel
{
public:
  static InputParameters validParams();

  AzimuthMagneticTimeDerivVectorRZ(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// Curl of the electric field vector
  const VectorVariableCurl & _efield_curl;
};
