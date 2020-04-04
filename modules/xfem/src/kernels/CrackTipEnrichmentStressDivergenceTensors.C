//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackTipEnrichmentStressDivergenceTensors.h"
#include "ElasticityTensorTools.h"

registerMooseObject("XFEMApp", CrackTipEnrichmentStressDivergenceTensors);

InputParameters
CrackTipEnrichmentStressDivergenceTensors::validParams()
{
  InputParameters params = ALEKernel::validParams();
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

CrackTipEnrichmentStressDivergenceTensors::CrackTipEnrichmentStressDivergenceTensors(
    const InputParameters & parameters)
  : ALEKernel(parameters),
    EnrichmentFunctionCalculation(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_base_name + "stress")),
    _Jacobian_mult(getMaterialPropertyByName<RankFourTensor>(_base_name + "Jacobian_mult")),
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

Real
CrackTipEnrichmentStressDivergenceTensors::computeQpResidual()
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

Real
CrackTipEnrichmentStressDivergenceTensors::computeQpJacobian()
{
  crackTipEnrichementFunctionAtPoint(*_current_elem->node_ptr(_i), _BI);
  crackTipEnrichementFunctionAtPoint(*_current_elem->node_ptr(_j), _BJ);

  crackTipEnrichementFunctionAtPoint(_q_point[_qp], _B);

  unsigned int crack_front_point_index =
      crackTipEnrichementFunctionDerivativeAtPoint(_q_point[_qp], _dBx);

  for (unsigned int i = 0; i < 4; ++i)
    rotateFromCrackFrontCoordsToGlobal(_dBx[i], _dBX[i], crack_front_point_index);

  RealVectorValue grad_B(_dBX[_enrichment_component]);

  RealVectorValue grad_test =
      _grad_test[_i][_qp] * (_B[_enrichment_component] - _BI[_enrichment_component]) +
      _test[_i][_qp] * grad_B;
  RealVectorValue grad_phi =
      _grad_phi[_j][_qp] * (_B[_enrichment_component] - _BJ[_enrichment_component]) +
      _phi[_j][_qp] * grad_B;

  return ElasticityTensorTools::elasticJacobian(
      _Jacobian_mult[_qp], _component, _component, grad_test, grad_phi);
}

Real
CrackTipEnrichmentStressDivergenceTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned int coupled_component = 0;
  unsigned int coupled_enrichment_component = 0;
  bool active(false);
  bool active_enrich(false);

  for (unsigned int i = 0; i < _enrich_disp_var.size(); ++i)
  {
    if (jvar == _enrich_disp_var[i])
    {
      coupled_component = i / 4;
      coupled_enrichment_component = i % 4;
      active_enrich = true;
    }
  }

  for (unsigned int i = 0; i < _disp_var.size(); ++i)
  {
    if (jvar == _disp_var[i])
    {
      coupled_component = i;
      active = true;
    }
  }

  if (active_enrich)
  {
    crackTipEnrichementFunctionAtPoint(*_current_elem->node_ptr(_i), _BI);
    crackTipEnrichementFunctionAtPoint(*_current_elem->node_ptr(_j), _BJ);

    crackTipEnrichementFunctionAtPoint(_q_point[_qp], _B);
    unsigned int crack_front_point_index =
        crackTipEnrichementFunctionDerivativeAtPoint(_q_point[_qp], _dBx);

    for (unsigned int i = 0; i < 4; ++i)
      rotateFromCrackFrontCoordsToGlobal(_dBx[i], _dBX[i], crack_front_point_index);

    RealVectorValue grad_B_test(_dBX[_enrichment_component]);
    RealVectorValue grad_B_phi(_dBX[coupled_enrichment_component]);

    RealVectorValue grad_test =
        _grad_test[_i][_qp] * (_B[_enrichment_component] - _BI[_enrichment_component]) +
        _test[_i][_qp] * grad_B_test;
    RealVectorValue grad_phi = _grad_phi[_j][_qp] * (_B[coupled_enrichment_component] -
                                                     _BJ[coupled_enrichment_component]) +
                               _phi[_j][_qp] * grad_B_phi;

    return ElasticityTensorTools::elasticJacobian(
        _Jacobian_mult[_qp], _component, coupled_component, grad_test, grad_phi);
  }
  else if (active)
  {
    crackTipEnrichementFunctionAtPoint(*_current_elem->node_ptr(_i), _BI);

    crackTipEnrichementFunctionAtPoint(_q_point[_qp], _B);
    unsigned int crack_front_point_index =
        crackTipEnrichementFunctionDerivativeAtPoint(_q_point[_qp], _dBx);

    for (unsigned int i = 0; i < 4; ++i)
      rotateFromCrackFrontCoordsToGlobal(_dBx[i], _dBX[i], crack_front_point_index);

    RealVectorValue grad_B_test(_dBX[_enrichment_component]);

    RealVectorValue grad_test =
        _grad_test[_i][_qp] * (_B[_enrichment_component] - _BI[_enrichment_component]) +
        _test[_i][_qp] * grad_B_test;

    return ElasticityTensorTools::elasticJacobian(
        _Jacobian_mult[_qp], _component, coupled_component, grad_test, _grad_phi[_j][_qp]);
  }

  return 0;
}
