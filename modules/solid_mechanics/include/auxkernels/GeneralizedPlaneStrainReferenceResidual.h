//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxScalarKernel.h"

// Forward Declarations
class GeneralizedPlaneStrainUserObject;

class GeneralizedPlaneStrainReferenceResidual : public AuxScalarKernel
{
public:
  static InputParameters validParams();

  GeneralizedPlaneStrainReferenceResidual(const InputParameters & parameters);

  virtual Real computeValue() override;

  const GeneralizedPlaneStrainUserObject & _gps;
  const unsigned int _scalar_var_id;
};
