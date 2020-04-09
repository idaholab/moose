//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowConstantBiotModulus.h"

registerMooseObject("PorousFlowApp", PorousFlowConstantBiotModulus);

InputParameters
PorousFlowConstantBiotModulus::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.addRangeCheckedParam<Real>(
      "biot_coefficient", 1.0, "biot_coefficient>=0 & biot_coefficient<=1", "Biot coefficient");
  params.addRangeCheckedParam<Real>(
      "fluid_bulk_modulus", 2.0E9, "fluid_bulk_modulus>0", "Fluid bulk modulus");
  params.addRangeCheckedParam<Real>("solid_bulk_compliance",
                                    0.0,
                                    "solid_bulk_compliance>=0.0",
                                    "Reciprocal of the drained bulk modulus of the porous "
                                    "skeleton.  If strain = C * stress, then solid_bulk_compliance "
                                    "= de_ij de_kl C_ijkl.  If the grain bulk modulus is Kg then "
                                    "1/Kg = (1 - biot_coefficient) * solid_bulk_compliance.");
  params.addPrivateParam<std::string>("pf_material_type", "biot_modulus");
  params.addClassDescription("Computes the Biot Modulus, which is assumed to be constant for all "
                             "time.  Sometimes 1 / BiotModulus is called storativity");
  return params;
}

PorousFlowConstantBiotModulus::PorousFlowConstantBiotModulus(const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _biot_coefficient(getParam<Real>("biot_coefficient")),
    _fluid_bulk_modulus(getParam<Real>("fluid_bulk_modulus")),
    _solid_bulk_compliance(getParam<Real>("solid_bulk_compliance")),
    _porosity(_nodal_material ? getMaterialProperty<Real>("PorousFlow_porosity_nodal")
                              : getMaterialProperty<Real>("PorousFlow_porosity_qp")),
    _biot_modulus(_nodal_material ? declareProperty<Real>("PorousFlow_constant_biot_modulus_nodal")
                                  : declareProperty<Real>("PorousFlow_constant_biot_modulus_qp")),
    _biot_modulus_old(_nodal_material
                          ? getMaterialPropertyOld<Real>("PorousFlow_constant_biot_modulus_nodal")
                          : getMaterialPropertyOld<Real>("PorousFlow_constant_biot_modulus_qp"))
{
}

void
PorousFlowConstantBiotModulus::initQpStatefulProperties()
{
  _biot_modulus[_qp] = 1.0 / ((1.0 - _biot_coefficient) * (_biot_coefficient - _porosity[_qp]) *
                                  _solid_bulk_compliance +
                              _porosity[_qp] / _fluid_bulk_modulus);
}

void
PorousFlowConstantBiotModulus::computeQpProperties()
{
  _biot_modulus[_qp] = _biot_modulus_old[_qp];
}
