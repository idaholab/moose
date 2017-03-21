/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "GeneralizedPlaneStrainOffDiag.h"
#include "Material.h"
#include "Assembly.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<GeneralizedPlaneStrainOffDiag>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Generalized Plane Strain kernel to provide contribution of the "
                             "out-of-plane strain to other kernels");
  params.addRequiredParam<std::vector<NonlinearVariableName>>("displacements",
                                                              "Variable for the displacements");
  params.addParam<NonlinearVariableName>("temperature", "Variable for the temperature");
  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar variable for generalized plane strain");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", "List of eigenstrains to be applied in this strain calculation");

  return params;
}

GeneralizedPlaneStrainOffDiag::GeneralizedPlaneStrainOffDiag(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _Jacobian_mult(getMaterialProperty<RankFourTensor>(_base_name + "Jacobian_mult")),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _deigenstrain_dT(_eigenstrain_names.size()),
    _scalar_out_of_plane_strain_var(coupledScalar("scalar_out_of_plane_strain")),
    _temp_var(isParamValid("temperature")
                  ? &_subproblem.getVariable(_tid, getParam<NonlinearVariableName>("temperature"))
                  : NULL)
{
  const std::vector<NonlinearVariableName> & nl_vnames(
      getParam<std::vector<NonlinearVariableName>>("displacements"));
  if (nl_vnames.size() > 2)
    mooseError("GeneralizedPlaneStrainOffDiag only works for 1D axisymmetric or 2D Cartesian "
               "generalized plane strain cases!");

  for (unsigned int i = 0; i < nl_vnames.size(); ++i)
    _disp_var.push_back(&_subproblem.getVariable(_tid, nl_vnames[i]));

  for (unsigned int i = 0; i < _deigenstrain_dT.size(); ++i)
    _deigenstrain_dT[i] = &getMaterialPropertyDerivative<RankTwoTensor>(
        _base_name + _eigenstrain_names[i], _temp_var->name());
}

void
GeneralizedPlaneStrainOffDiag::computeOffDiagJacobianScalar(unsigned int jvar)
{
  if (_assembly.coordSystem() == Moose::COORD_XYZ)
    _scalar_out_of_plane_strain_direction = 2;
  else if (_assembly.coordSystem() == Moose::COORD_RZ)
    _scalar_out_of_plane_strain_direction = 1;

  if (_var.number() == _disp_var[0]->number())
    computeDispOffDiagJacobianScalar(0, jvar);
  else if (_var.number() == _disp_var[1]->number())
    computeDispOffDiagJacobianScalar(1, jvar);
  else if (isParamValid("temperature") ? _var.number() == _temp_var->number() : 0)
    computeTempOffDiagJacobianScalar(jvar);
}

void
GeneralizedPlaneStrainOffDiag::computeDispOffDiagJacobianScalar(unsigned int component,
                                                                unsigned int jvar)
{
  if (jvar == _scalar_out_of_plane_strain_var)
  {
    DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), jvar);
    DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar, _var.number());
    MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);

    for (_i = 0; _i < _test.size(); ++_i)
      for (_j = 0; _j < jv.order(); ++_j)
        for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
        {
          ken(_i, _j) += _JxW[_qp] * _coord[_qp] *
                         _Jacobian_mult[_qp](_scalar_out_of_plane_strain_direction,
                                             _scalar_out_of_plane_strain_direction,
                                             component,
                                             component) *
                         _grad_test[_i][_qp](component);
          kne(_j, _i) += _JxW[_qp] * _coord[_qp] *
                         _Jacobian_mult[_qp](_scalar_out_of_plane_strain_direction,
                                             _scalar_out_of_plane_strain_direction,
                                             component,
                                             component) *
                         _grad_test[_i][_qp](component);
        }
  }
}

void
GeneralizedPlaneStrainOffDiag::computeTempOffDiagJacobianScalar(unsigned int jvar)
{
  if (jvar == _scalar_out_of_plane_strain_var)
  {
    DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar, _var.number());
    MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);
    unsigned int n_eigenstrains = _deigenstrain_dT.size();

    for (_i = 0; _i < _test.size(); ++_i)
      for (_j = 0; _j < jv.order(); ++_j)
        for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
          for (unsigned int ies = 0; ies < n_eigenstrains; ++ies)
            kne(_j, _i) +=
                _JxW[_qp] * _coord[_qp] *
                (_Jacobian_mult[_qp] * (*_deigenstrain_dT[ies])[_qp])(
                    _scalar_out_of_plane_strain_direction, _scalar_out_of_plane_strain_direction) *
                _test[_i][_qp];
  }
}
