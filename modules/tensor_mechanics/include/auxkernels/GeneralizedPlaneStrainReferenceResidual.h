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
class GeneralizedPlaneStrainReferenceResidual;
class GeneralizedPlaneStrainUserObject;

template <>
InputParameters validParams<GeneralizedPlaneStrainReferenceResidual>();

class GeneralizedPlaneStrainReferenceResidual : public AuxScalarKernel
{
public:
  GeneralizedPlaneStrainReferenceResidual(const InputParameters & parameters);

  virtual Real computeValue() override;

  const GeneralizedPlaneStrainUserObject & _gps;
  const unsigned int _scalar_var_id;
};
