/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowVolumetricStrain.h"
#include "MooseMesh.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<PorousFlowVolumetricStrain>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
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
  params.set<bool>("stateful_displacements") = true;
  params.set<bool>("at_nodes") = false;
  return params;
}

PorousFlowVolumetricStrain::PorousFlowVolumetricStrain(const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _consistent(getParam<bool>("consistent_with_displaced_mesh")),
    _ndisp(coupledComponents("displacements")),
    _disp(3),
    _disp_var_num(3),
    _grad_disp(3),
    _grad_disp_old(3),

    _vol_strain_rate_qp(declareProperty<Real>("PorousFlow_volumetric_strain_rate_qp")),
    _dvol_strain_rate_qp_dvar(
        declareProperty<std::vector<RealGradient>>("dPorousFlow_volumetric_strain_rate_qp_dvar")),
    _vol_total_strain_qp(declareProperty<Real>("PorousFlow_total_volumetric_strain_qp")),
    _dvol_total_strain_qp_dvar(
        declareProperty<std::vector<RealGradient>>("dPorousFlow_total_volumetric_strain_qp_dvar"))
{
  if (_ndisp != _mesh.dimension())
    mooseError("PorousFlowVolumetricStrain: The number of variables supplied in 'displacements' "
               "must match the mesh dimension.");

  // fetch coupled variables and gradients (as stateful properties if necessary)
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp[i] = &coupledValue("displacements", i);
    _disp_var_num[i] = coupled("displacements", i);
    _grad_disp[i] = &coupledGradient("displacements", i);
    _grad_disp_old[i] = &coupledGradientOld("displacements", i);
  }

  // set unused dimensions to zero
  for (unsigned i = _ndisp; i < 3; ++i)
  {
    _disp[i] = &_zero;
    _disp_var_num[i] = 0;
    while (_dictator.isPorousFlowVariable(_disp_var_num[i]))
      _disp_var_num[i]++; // increment until disp_var_num[i] is not a porflow var
    _grad_disp[i] = &_grad_zero;
    _grad_disp_old[i] = &_grad_zero;
  }
  if (_nodal_material == true)
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
  const Real andy = (_consistent
                         ? 1.0 + (*_grad_disp_old[0])[_qp](0) + (*_grad_disp_old[1])[_qp](1) +
                               (*_grad_disp_old[2])[_qp](2)
                         : 1.0);
  _vol_strain_rate_qp[_qp] = total_strain_increment.trace() / _dt / andy;

  // prepare the derivatives with zeroes
  _dvol_strain_rate_qp_dvar[_qp].resize(_num_var, RealGradient());
  _dvol_total_strain_qp_dvar[_qp].resize(_num_var, RealGradient());
  for (unsigned i = 0; i < _ndisp; ++i)
    if (_dictator.isPorousFlowVariable(_disp_var_num[i]))
    {
      // the i_th displacement is a porous-flow variable
      const unsigned int pvar = _dictator.porousFlowVariableNum(_disp_var_num[i]);
      _dvol_strain_rate_qp_dvar[_qp][pvar](i) = 1.0 / _dt / andy;
      _dvol_total_strain_qp_dvar[_qp][pvar](i) = 1.0;
    }
}
