//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCrackTipEnrichmentStressDivergenceTensors.h"
#include "ElasticityTensorTools.h"

registerMooseObject("XFEMApp", ADCrackTipEnrichmentStressDivergenceTensors);

InputParameters
ADCrackTipEnrichmentStressDivergenceTensors::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Enrich stress divergence kernel for small-strain simulations");
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction the variable "
                                        "this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addRequiredParam<unsigned int>("enrichment_component",
                                        "The component of the enrichement functions");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.addRequiredCoupledVar(
      "enrichment_displacements",
      "The string of enrichment displacements suitable for the problem statement");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addRequiredParam<UserObjectName>("crack_front_definition",
                                          "The CrackFrontDefinition user object name");
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

ADCrackTipEnrichmentStressDivergenceTensors::ADCrackTipEnrichmentStressDivergenceTensors(
    const InputParameters & parameters)
  : ADKernel(parameters),
    EnrichmentFunctionCalculation(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(getADMaterialPropertyByName<RankTwoTensor>(_base_name + "stress")),
    // _Jacobian_mult(getADMaterialPropertyByName<RankFourTensor>(_base_name + "Jacobian_mult")),
    _component(getParam<unsigned int>("component")),
    _enrichment_component(getParam<unsigned int>("enrichment_component")),
    _nenrich_disp(coupledComponents("enrichment_displacements")),
    _ndisp(coupledComponents("displacements")),
    _B(4),
    _dBX(4),
    _dBx(4),
    _BI(4),
    _BJ(4)
{
  _enrich_disp_var.resize(_nenrich_disp);
  for (unsigned int i = 0; i < _nenrich_disp; ++i)
    _enrich_disp_var[i] = coupled("enrichment_displacements", i);

  _disp_var.resize(_ndisp);
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var[i] = coupled("displacements", i);
}

ADReal
ADCrackTipEnrichmentStressDivergenceTensors::computeQpResidual()
{
  crackTipEnrichementFunctionAtPoint(*_current_elem->node_ptr(_i), _BI);

  crackTipEnrichementFunctionAtPoint(_q_point[_qp], _B);
  unsigned int crack_front_point_index =
      crackTipEnrichementFunctionDerivativeAtPoint(_q_point[_qp], _dBx);

  for (unsigned int i = 0; i < 4; ++i)
    rotateFromCrackFrontCoordsToGlobal(_dBx[i], _dBX[i], crack_front_point_index);

  RealVectorValue grad_B(_dBX[_enrichment_component]);

  return _stress[_qp].row(_component) *
         (_grad_test[_i][_qp] * (_B[_enrichment_component] - _BI[_enrichment_component]) +
          _test[_i][_qp] * grad_B);
}
