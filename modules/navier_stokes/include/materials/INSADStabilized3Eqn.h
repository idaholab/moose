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

  const ADVariableSecond & _second_temperature;

  const ADMaterialProperty<Real> & _k;
  const ADMaterialProperty<RealVectorValue> * const _grad_k;

  ADMaterialProperty<Real> & _tau_energy;
  ADMaterialProperty<Real> & _temperature_strong_residual;

  using INSADTauMaterialTempl<INSAD3Eqn>::_cp;
  using INSADTauMaterialTempl<INSAD3Eqn>::_temperature_advective_strong_residual;
  using INSADTauMaterialTempl<INSAD3Eqn>::_temperature_td_strong_residual;
  using INSADTauMaterialTempl<INSAD3Eqn>::_temperature_ambient_convection_strong_residual;
  using INSADTauMaterialTempl<INSAD3Eqn>::_temperature_source_strong_residual;
  using INSADTauMaterialTempl<INSAD3Eqn>::_has_ambient_convection;
  using INSADTauMaterialTempl<INSAD3Eqn>::_has_heat_source;
  using INSADTauMaterialTempl<INSAD3Eqn>::_has_energy_transient;
};
