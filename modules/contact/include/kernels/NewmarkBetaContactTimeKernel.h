//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"
#include "Material.h"

// Forward Declarations
class TimeIntegrator;

class NewmarkBetaContactTimeKernel : public ADKernel
{
public:
  static InputParameters validParams();

  NewmarkBetaContactTimeKernel(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  const MaterialProperty<Real> & _density;
  const VariableValue & _u_old;
};
