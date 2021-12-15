//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeCrackTipEnrichmentSmallStrain.h"
#include "MooseMesh.h"
#include "libmesh/fe_interface.h"
#include "libmesh/string_to_enum.h"

#include "libmesh/quadrature_gauss.h"

registerMooseObject("XFEMApp", ComputeCrackTipEnrichmentSmallStrain);

InputParameters
ComputeCrackTipEnrichmentSmallStrain::validParams()
{
  InputParameters params = ComputeStrainBase::validParams();
  params.addClassDescription(
      "Computes the crack tip enrichment at a point within a small strain formulation.");
  params.addRequiredParam<std::vector<NonlinearVariableName>>("enrichment_displacements",
                                                              "The enrichment displacement");
  params.addRequiredParam<UserObjectName>("crack_front_definition",
                                          "The CrackFrontDefinition user object name");
  return params;
}

ComputeCrackTipEnrichmentSmallStrain::ComputeCrackTipEnrichmentSmallStrain(
    const InputParameters & parameters)
  : ComputeStrainBase(parameters),
    EnrichmentFunctionCalculation(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _enrich_disp(3),
    _grad_enrich_disp(3),
    _enrich_variable(4),
    _phi(_assembly.phi()),
    _grad_phi(_assembly.gradPhi()),
    _B(4),
    _dBX(4),
    _dBx(4)
{
  for (unsigned int i = 0; i < _enrich_variable.size(); ++i)
    _enrich_variable[i].resize(_ndisp);

  const std::vector<NonlinearVariableName> & nl_vnames =
      getParam<std::vector<NonlinearVariableName>>("enrichment_displacements");

  if (_ndisp == 2 && nl_vnames.size() != 8)
    mooseError("The number of enrichment displacements should be total 8 for 2D.");
  else if (_ndisp == 3 && nl_vnames.size() != 12)
    mooseError("The number of enrichment displacements should be total 12 for 3D.");

  NonlinearSystem & nl = _fe_problem.getNonlinearSystem();
  _nl = &(_fe_problem.getNonlinearSystem());

  for (unsigned int j = 0; j < _ndisp; ++j)
    for (unsigned int i = 0; i < 4; ++i)
      _enrich_variable[i][j] = &(nl.getVariable(0, nl_vnames[j * 4 + i]));

  if (_ndisp == 2)
    _BI.resize(4); // QUAD4
  else if (_ndisp == 3)
    _BI.resize(8); // HEX8

  for (unsigned int i = 0; i < _BI.size(); ++i)
    _BI[i].resize(4);
}

void
ComputeCrackTipEnrichmentSmallStrain::computeQpProperties()
{
  crackTipEnrichementFunctionAtPoint(_q_point[_qp], _B);
  unsigned int crack_front_point_index =
      crackTipEnrichementFunctionDerivativeAtPoint(_q_point[_qp], _dBx);

  for (unsigned int i = 0; i < 4; ++i)
    rotateFromCrackFrontCoordsToGlobal(_dBx[i], _dBX[i], crack_front_point_index);

  _sln = _nl->currentSolution();

  for (unsigned int m = 0; m < _ndisp; ++m)
  {
    _enrich_disp[m] = 0.0;
    _grad_enrich_disp[m].zero();
    for (unsigned int i = 0; i < _current_elem->n_nodes(); ++i)
    {
      const Node * node_i = _current_elem->node_ptr(i);
      for (unsigned int j = 0; j < 4; ++j)
      {
        dof_id_type dof = node_i->dof_number(_nl->number(), _enrich_variable[j][m]->number(), 0);
        Real soln = (*_sln)(dof);
        _enrich_disp[m] += (*_fe_phi)[i][_qp] * (_B[j] - _BI[i][j]) * soln;
        RealVectorValue grad_B(_dBX[j]);
        _grad_enrich_disp[m] +=
            ((*_fe_dphi)[i][_qp] * (_B[j] - _BI[i][j]) + (*_fe_phi)[i][_qp] * grad_B) * soln;
      }
    }
  }

  auto grad_tensor_enrich = RankTwoTensor::initializeFromRows(
      _grad_enrich_disp[0], _grad_enrich_disp[1], _grad_enrich_disp[2]);

  RankTwoTensor enrich_strain = (grad_tensor_enrich + grad_tensor_enrich.transpose()) / 2.0;

  auto grad_tensor = RankTwoTensor::initializeFromRows(
      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);

  _total_strain[_qp] = (grad_tensor + grad_tensor.transpose()) / 2.0;

  _total_strain[_qp] += enrich_strain;

  _mechanical_strain[_qp] = _total_strain[_qp];

  // Remove the Eigen strain
  for (auto es : _eigenstrains)
    _mechanical_strain[_qp] -= (*es)[_qp];
}

void
ComputeCrackTipEnrichmentSmallStrain::computeProperties()
{
  FEType fe_type(Utility::string_to_enum<Order>("first"),
                 Utility::string_to_enum<FEFamily>("lagrange"));
  const unsigned int dim = _current_elem->dim();
  std::unique_ptr<FEBase> fe(FEBase::build(dim, fe_type));
  fe->attach_quadrature_rule(const_cast<QBase *>(_qrule));
  _fe_phi = &(fe->get_phi());
  _fe_dphi = &(fe->get_dphi());

  if (isBoundaryMaterial())
    fe->reinit(_current_elem, _current_side);
  else
    fe->reinit(_current_elem);

  for (unsigned int i = 0; i < _BI.size(); ++i)
    crackTipEnrichementFunctionAtPoint(*(_current_elem->node_ptr(i)), _BI[i]);

  ComputeStrainBase::computeProperties();
}
