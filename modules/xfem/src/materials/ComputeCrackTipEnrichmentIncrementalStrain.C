//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeCrackTipEnrichmentIncrementalStrain.h"
#include "MooseMesh.h"
#include "libmesh/fe_interface.h"
#include "libmesh/string_to_enum.h"

#include "libmesh/quadrature.h"

registerMooseObject("XFEMApp", ComputeCrackTipEnrichmentIncrementalStrain);

InputParameters
ComputeCrackTipEnrichmentIncrementalStrain::validParams()
{
  InputParameters params = ComputeIncrementalStrainBase::validParams();
  params.addClassDescription(
      "Computes the crack tip enrichment strain for an incremental small-strain formulation.");
  params.addRequiredParam<std::vector<NonlinearVariableName>>("enrichment_displacements",
                                                              "The enrichment displacement");

  params.addRequiredParam<UserObjectName>("crack_front_definition",
                                          "The CrackFrontDefinition user object name");

  return params;
}

ComputeCrackTipEnrichmentIncrementalStrain::ComputeCrackTipEnrichmentIncrementalStrain(
    const InputParameters & parameters)
  : ComputeIncrementalStrainBase(parameters),
    EnrichmentFunctionCalculation(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _enrich_disp(3),
    _grad_enrich_disp(3),
    _grad_enrich_disp_old(3),
    _enrich_variable(4),
    _phi(_assembly.phi()),
    _grad_phi(_assembly.gradPhi()),
    _grad_enrich_disp_tensor(
        declareProperty<RankTwoTensor>(_base_name + "grad_enrich_disp_tensor")),
    _grad_enrich_disp_tensor_old(
        getMaterialPropertyOld<RankTwoTensor>(_base_name + "grad_enrich_disp_tensor")),
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

  _nl = &(_fe_problem.getNonlinearSystem(/*nl_sys_num=*/0));

  for (unsigned int j = 0; j < _ndisp; ++j)
    for (unsigned int i = 0; i < 4; ++i)
      _enrich_variable[i][j] = &(_nl->getVariable(0, nl_vnames[j * 4 + i]));

  if (_ndisp == 2)
    _BI.resize(4); // QUAD4
  else if (_ndisp == 3)
    _BI.resize(8); // HEX8

  for (unsigned int i = 0; i < _BI.size(); ++i)
    _BI[i].resize(4);
}

void
ComputeCrackTipEnrichmentIncrementalStrain::computeProperties()
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

  _sln = _nl->currentSolution();

  for (unsigned int i = 0; i < _BI.size(); ++i)
    crackTipEnrichementFunctionAtPoint(*(_current_elem->node_ptr(i)), _BI[i]);

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // enrichment function
    crackTipEnrichementFunctionAtPoint(_q_point[_qp], _B);
    unsigned int crack_front_point_index =
        crackTipEnrichementFunctionDerivativeAtPoint(_q_point[_qp], _dBx);

    for (unsigned int i = 0; i < 4; ++i)
      rotateFromCrackFrontCoordsToGlobal(_dBx[i], _dBX[i], crack_front_point_index);

    for (unsigned int m = 0; m < _ndisp; ++m)
    {
      _enrich_disp[m] = 0.0;
      _grad_enrich_disp[m].zero();
      _grad_enrich_disp_old[m].zero();
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

    _grad_enrich_disp_tensor[_qp] = RankTwoTensor::initializeFromRows(
        _grad_enrich_disp[0], _grad_enrich_disp[1], _grad_enrich_disp[2]);

    auto grad_disp_tensor = RankTwoTensor::initializeFromRows(
        (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);

    auto grad_disp_tensor_old = RankTwoTensor::initializeFromRows(
        (*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]);

    _deformation_gradient[_qp] = grad_disp_tensor + _grad_enrich_disp_tensor[_qp];
    _deformation_gradient[_qp].addIa(1.0);

    _strain_increment[_qp] =
        0.5 * ((grad_disp_tensor + _grad_enrich_disp_tensor[_qp]) +
               (grad_disp_tensor + _grad_enrich_disp_tensor[_qp]).transpose()) -
        0.5 * ((grad_disp_tensor_old + _grad_enrich_disp_tensor_old[_qp]) +
               (grad_disp_tensor_old + _grad_enrich_disp_tensor_old[_qp]).transpose());

    // total strain
    _total_strain[_qp] = _total_strain_old[_qp] + _strain_increment[_qp];

    // Remove the Eigen strain increment
    subtractEigenstrainIncrementFromStrain(_strain_increment[_qp]);

    // strain rate
    if (_dt > 0)
    {
      _strain_rate[_qp] = _strain_increment[_qp] / _dt;
      _grad_disp_rate[_qp] = (grad_disp_tensor - grad_disp_tensor_old) / _dt;
    }
    else
    {
      _strain_rate[_qp].zero();
      _grad_disp_rate[_qp].zero();
    }

    // Update strain in intermediate configuration: rotations are not needed
    _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];

    // incremental small strain does not include rotation
    _rotation_increment[_qp].setToIdentity();
  }
}
