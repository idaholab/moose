//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"

class DragCoefficients;

declareADValidParams(DragCoefficients);

/**
 * Abstract base class material providing the drag coefficients
 * proportional to velocity (Darcy coefficient) and proportional to the
 * square of velocity (Forchheimer coefficient). The drag term in the
 * momentum equation has a strong form \f$W\rho_f\vec{V}\f$, where
 * \f$W\equiv C_L+C_Q\f$, where \f$C_L\f$ is the linear drag coefficient
 * and \f$C_Q\f$ is the quadratic drag coefficient.
 */
class DragCoefficients : public ADMaterial
{
public:
  DragCoefficients(const InputParameters & parameters);

protected:
  /// linear drag coefficient
  ADMaterialProperty<RealVectorValue> & _cL;

  /// quadratic drag coefficient
  ADMaterialProperty<RealVectorValue> & _cQ;

};
