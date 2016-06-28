/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StressDivergenceTensors.h"
#include "Material.h"
#include "MooseMesh.h"
#include "ElasticityTensorTools.h"
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<StressDivergenceTensors>()
{
  InputParameters params = validParams<ALEKernel>();
  params.addClassDescription("Stress divergence kernel (used by the TensorMechanics action)");
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addRequiredCoupledVar("displacements", "The string of displacements suitable for the problem statement");
  params.addCoupledVar("temp", "The temperature");
  params.addParam<std::string>("base_name", "Material property base name");
  params.set<bool>("use_displaced_mesh") = false;
  params.addParam<bool>("use_finite_deform_jacobian", false, "Jacobian for corotational finite strain");
  return params;
}


StressDivergenceTensors::StressDivergenceTensors(const InputParameters & parameters) :
    ALEKernel(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _use_finite_deform_jacobian(getParam<bool>("use_finite_deform_jacobian")),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_base_name + "stress")),
    _Jacobian_mult(getMaterialPropertyByName<RankFourTensor>(_base_name + "Jacobian_mult")),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_var(3),
    _temp_coupled(isCoupled("temp")),
    _temp_var(_temp_coupled ? coupled("temp") : 0)
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var[i] = coupled("displacements", i);

  // Checking for consistency between mesh size and length of the provided displacements vector
  if (_ndisp != _mesh.dimension())
    mooseError("The number of displacement variables supplied must match the mesh dimension.");

  if (_use_finite_deform_jacobian)
  {
    _deformation_gradient = &getMaterialProperty<RankTwoTensor>(_base_name + "deformation_gradient");
    _deformation_gradient_old = &getMaterialPropertyOld<RankTwoTensor>(_base_name + "deformation_gradient");
    _rotation_increment = &getMaterialProperty<RankTwoTensor>(_base_name + "rotation_increment");
  }
}

Real
StressDivergenceTensors::computeQpResidual()
{
  return _stress[_qp].row(_component) * _grad_test[_i][_qp];
}

void
StressDivergenceTensors::computeJacobian()
{
  if (_use_finite_deform_jacobian)
  {
    _finite_deform_Jacobian_mult.resize(_qrule->n_points());

    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      computeFiniteDeformJacobian();

    ALEKernel::computeJacobian();
  }
  else
    Kernel::computeJacobian();
}

void
StressDivergenceTensors::computeOffDiagJacobian(unsigned int jvar)
{
  if (_use_finite_deform_jacobian)
  {
    _finite_deform_Jacobian_mult.resize(_qrule->n_points());

    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      computeFiniteDeformJacobian();

    ALEKernel::computeOffDiagJacobian(jvar);
  }
  else
    Kernel::computeOffDiagJacobian(jvar);
}

Real
StressDivergenceTensors::computeQpJacobian()
{
  if (_use_finite_deform_jacobian)
    return ElasticityTensorTools::elasticJacobian(_finite_deform_Jacobian_mult[_qp], _component, _component, _grad_test[_i][_qp], _grad_phi_undisplaced[_j][_qp]);

  return ElasticityTensorTools::elasticJacobian(_Jacobian_mult[_qp], _component, _component, _grad_test[_i][_qp], _grad_phi[_j][_qp]);
}

Real
StressDivergenceTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned int coupled_component = 0;
  bool active(false);

  for (unsigned int i = 0; i < _ndisp; ++i)
    if (jvar == _disp_var[i])
    {
      coupled_component = i;
      active = true;
    }

  if (active)
  {
    if (_use_finite_deform_jacobian)
      return ElasticityTensorTools::elasticJacobian(_finite_deform_Jacobian_mult[_qp], _component, coupled_component,
                                            _grad_test[_i][_qp], _grad_phi_undisplaced[_j][_qp]);

    return ElasticityTensorTools::elasticJacobian(_Jacobian_mult[_qp], _component, coupled_component,
                                          _grad_test[_i][_qp], _grad_phi[_j][_qp]);
  }

  if (_temp_coupled && jvar == _temp_var)
  {
    //return _d_stress_dT[_qp].rowDot(_component, _grad_test[_i][_qp]) * _phi[_j][_qp];
    return 0.0;
  }

  return 0;
}

void
StressDivergenceTensors::computeFiniteDeformJacobian()
{
  RankTwoTensor identity(RankTwoTensor::initIdentity);
  RankTwoTensor unrotated_stress = (*_rotation_increment)[_qp].transpose() * _stress[_qp] * (*_rotation_increment)[_qp];//Bring back to unrotated config
  RankTwoTensor Fhat = (*_deformation_gradient)[_qp] * (*_deformation_gradient_old)[_qp].inverse();//Incremental deformation gradient Fhat
  RankTwoTensor Fhatinv = Fhat.inverse();//Fhat inverse

  RankTwoTensor rot_times_stress = (*_rotation_increment)[_qp] * unrotated_stress;
  RankFourTensor dstress_drot = identity.mixedProductIkJl(rot_times_stress) + identity.mixedProductJkIl(rot_times_stress);
  RankFourTensor rot_rank_four = (*_rotation_increment)[_qp].mixedProductIkJl((*_rotation_increment)[_qp]);
  RankFourTensor drot_dUhatinv = Fhat.mixedProductIkJl(identity);

  RankTwoTensor A(RankTwoTensor::initIdentity);
  A -= Fhatinv;

  //Ctilde = Chat^-1 - I
  RankTwoTensor Ctilde = A * A.transpose() - A - A.transpose();
  RankFourTensor dCtilde_dFhatinv = -identity.mixedProductIkJl(A) - identity.mixedProductJkIl(A) + identity.mixedProductIkJl(identity) + identity.mixedProductJkIl(identity);

  RankTwoTensor Uhat(RankTwoTensor::initIdentity);
  Uhat -= 0.5 * Ctilde + 3.0/8.0 * Ctilde * Ctilde;//Second order approximation of Uhat - consistent with strain increment definition

  RankFourTensor dUhatinv_dCtilde = 0.5 * identity.mixedProductIkJl(identity) - 1.0/8.0 * (identity.mixedProductIkJl(Ctilde) + Ctilde.mixedProductIkJl(identity));
  RankFourTensor drot_dFhatinv = drot_dUhatinv * dUhatinv_dCtilde * dCtilde_dFhatinv;

  drot_dFhatinv -= Fhat.mixedProductIkJl((*_rotation_increment)[_qp].transpose());
  _finite_deform_Jacobian_mult[_qp] = dstress_drot * drot_dFhatinv;


  RankFourTensor dstrain_increment_dCtilde = -0.5 * identity.mixedProductIkJl(identity) + 0.25 * (identity.mixedProductIkJl(Ctilde) + Ctilde.mixedProductIkJl(identity));
  _finite_deform_Jacobian_mult[_qp] += rot_rank_four * _Jacobian_mult[_qp] * dstrain_increment_dCtilde * dCtilde_dFhatinv;
  _finite_deform_Jacobian_mult[_qp] += Fhat.mixedProductJkIl(_stress[_qp]);

  RankFourTensor dFhat_dFhatinv = -Fhat.mixedProductIkJl(Fhat.transpose());
  RankTwoTensor dJ_dFhatinv = dFhat_dFhatinv.innerProductTranspose(Fhat.ddet());

  //Component from Jacobian derivative
  _finite_deform_Jacobian_mult[_qp] += _stress[_qp].outerProduct(dJ_dFhatinv);

  //Derivative of Fhatinv w.r.t. undisplaced coordinates
  RankTwoTensor Finv = (*_deformation_gradient)[_qp].inverse();
  RankFourTensor dFhatinv_dGradu = -Fhatinv.mixedProductIkJl(Finv.transpose());
  _finite_deform_Jacobian_mult[_qp] = _finite_deform_Jacobian_mult[_qp] * dFhatinv_dGradu;
}
