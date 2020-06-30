//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "NonlinearSystemBase.h"
#include "FEProblemBase.h"
#include "MaterialProperty.h"
#include "MooseArray.h"

#include "libmesh/elem.h"

#include <vector>

class INSADMaterial;
class INSAD3Eqn;

#include "INSADTauMaterial.h"
#include "INSAD3Eqn.h"

class INSADStabilized3Eqn : public INSADTauMaterialTempl<INSAD3Eqn>
{
public:
  static InputParameters validParams();

  INSADStabilized3Eqn(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  ADMaterialProperty<Real> & _tau_energy;

  using INSADTauMaterialTempl<INSAD3Eqn>::_k;
  using INSADTauMaterialTempl<INSAD3Eqn>::_cp;
};
