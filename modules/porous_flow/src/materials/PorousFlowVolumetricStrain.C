//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowVolumetricStrain.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

registerMooseObject("PorousFlowApp", PorousFlowVolumetricStrain);

InputParameters
PorousFlowVolumetricStrain::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<bool>("consistent_with_displaced_mesh",
                        true,
                        "The volumetric strain rate will "
                        "include terms that ensure fluid "
                        "mass conservation in the "
                        "displaced mesh");
  params.addClassDescription(
      "Compute volumetric strain and the volumetric_strain rate, for use in PorousFlow.");
  params.set<std::string>("pf_material_type") = "volumetric_strain";
  params.set<bool>("stateful_displacements") = true;
  params.set<bool>("at_nodes") = false;
  return params;
}

PorousFlowVolumetricStrain::PorousFlowVolumetricStrain(const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _consistent(getParam<bool>("consistent_with_displaced_mesh")),
    _ndisp(coupledComponents("displacements")),
    _disp(coupledValues("displacements")),
    _disp_var_num(coupledIndices("displacements")),
    _grad_disp(coupledGradients("displacements")),
    _grad_disp_old(coupledGradientsOld("displacements")),

    _vol_strain_rate_qp(declareProperty<Real>("PorousFlow_volumetric_strain_rate_qp")),
    _dvol_strain_rate_qp_dvar(
        declareProperty<std::vector<RealGradient>>("dPorousFlow_volumetric_strain_rate_qp_dvar")),
    _vol_total_strain_qp(declareProperty<Real>("PorousFlow_total_volumetric_strain_qp")),
    _dvol_total_strain_qp_dvar(
        declareProperty<std::vector<RealGradient>>("dPorousFlow_total_volumetric_strain_qp_dvar"))
{
  if (_ndisp != _mesh.dimension())
    paramError("displacements", "The number of variables supplied must match the mesh dimension.");

  // set unused dimensions to zero
  _disp.resize(3, &_zero);
  _disp_var_num.resize(3, 0);
  _grad_disp.resize(3, &_grad_zero);
  _grad_disp_old.resize(3, &_grad_zero);
  for (unsigned i = _ndisp; i < 3; ++i)
    while (_dictator.isPorousFlowVariable(_disp_var_num[i]))
      _disp_var_num[i]++; // increment until disp_var_num[i] is not a porflow var

  if (_nodal_material)
    mooseError("PorousFlowVolumetricStrain classes are only defined for at_nodes = false");
}

void
PorousFlowVolumetricStrain::initQpStatefulProperties()
{
  _vol_total_strain_qp[_qp] = 0.0;
}

void
PorousFlowVolumetricStrain::computeQpProperties()
{
  RankTwoTensor A(
      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]); // Deformation gradient
  RankTwoTensor Fbar((*_grad_disp_old[0])[_qp],
                     (*_grad_disp_old[1])[_qp],
                     (*_grad_disp_old[2])[_qp]); // Old Deformation gradient

  _vol_total_strain_qp[_qp] = A.trace();

  A -= Fbar; // A = grad_disp - grad_disp_old

  RankTwoTensor total_strain_increment = 0.5 * (A + A.transpose());
  const Real andy = (_consistent ? 1.0 + (*_grad_disp_old[0])[_qp](0) +
                                       (*_grad_disp_old[1])[_qp](1) + (*_grad_disp_old[2])[_qp](2)
                                 : 1.0);
  _vol_strain_rate_qp[_qp] = total_strain_increment.trace() / _dt / andy;

  // prepare the derivatives with zeroes
  _dvol_strain_rate_qp_dvar[_qp].resize(_num_var, RealGradient());
  _dvol_total_strain_qp_dvar[_qp].resize(_num_var, RealGradient());
  for (unsigned i = 0; i < _ndisp; ++i)
    if (_dictator.isPorousFlowVariable(_disp_var_num[i]))
    {
      // the i_th displacement is a PorousFlow variable
      const unsigned int pvar = _dictator.porousFlowVariableNum(_disp_var_num[i]);
      _dvol_strain_rate_qp_dvar[_qp][pvar](i) = 1.0 / _dt / andy;
      _dvol_total_strain_qp_dvar[_qp][pvar](i) = 1.0;
    }
}
