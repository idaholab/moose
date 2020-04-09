//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SplitCHCRes.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations

/**
 * CHParsed uses the Free Energy function and derivatives
 * provided by a DerivativeParsedMaterial.
 * This is the split operator variant.
 * \see CHParsed
 */
class SplitCHParsed : public DerivativeMaterialInterface<JvarMapKernelInterface<SplitCHCRes>>
{
public:
  static InputParameters validParams();

  SplitCHParsed(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual Real computeDFDC(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const MaterialProperty<Real> & _dFdc;
  const MaterialProperty<Real> & _d2Fdc2;
  std::vector<const MaterialProperty<Real> *> _d2Fdcdarg;
};
