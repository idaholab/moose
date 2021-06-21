//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVHLLCBCBase.h"
#include "CNSFVHLLC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"

InputParameters
CNSFVHLLCBCBase::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "Fluid properties userobject");
  return params;
}

CNSFVHLLCBCBase::CNSFVHLLCBCBase(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _specific_internal_energy_elem(getADMaterialProperty<Real>(NS::specific_internal_energy)),
    _vel_elem(getADMaterialProperty<RealVectorValue>(NS::velocity)),
    _speed_elem(getADMaterialProperty<Real>(NS::speed)),
    _rho_elem(getADMaterialProperty<Real>(NS::density)),
    _pressure_elem(getADMaterialProperty<Real>(NS::pressure)),
    _rho_et_elem(getADMaterialProperty<Real>(NS::total_energy_density)),
    _ht_elem(getADMaterialProperty<Real>(NS::specific_total_enthalpy))
{
}

HLLCData
CNSFVHLLCBCBase::hllcData() const
{
  return {_fluid,
          _rho_elem[_qp],
          _rho_boundary,
          _vel_elem[_qp],
          _vel_boundary,
          _specific_internal_energy_elem[_qp],
          _specific_internal_energy_boundary};
}
