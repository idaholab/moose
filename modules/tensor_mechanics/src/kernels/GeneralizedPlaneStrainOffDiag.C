//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedPlaneStrainOffDiag.h"

// MOOSE includes
#include "Assembly.h"
#include "Material.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

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
  params.addParam<UserObjectName>("subblock_index_provider",
                                  "SubblockIndexProvider user object name");
  params.addParam<unsigned int>(
      "scalar_out_of_plane_strain_index",
      "The index number of scalar_out_of_plane_strain this kernel acts on");
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
    _subblock_id_provider(isParamValid("subblock_index_provider")
                              ? &getUserObject<SubblockIndexProvider>("subblock_index_provider")
                              : nullptr),
    _scalar_var_id(isParamValid("scalar_out_of_plane_strain_index")
                       ? getParam<unsigned int>("scalar_out_of_plane_strain_index")
                       : 0),
    _temp_var(isParamValid("temperature")
                  ? &_subproblem.getVariable(_tid, getParam<NonlinearVariableName>("temperature"))
                  : NULL),
    _num_disp_var(getParam<std::vector<NonlinearVariableName>>("displacements").size())
{
  const std::vector<NonlinearVariableName> & nl_vnames(
      getParam<std::vector<NonlinearVariableName>>("displacements"));
  if (_num_disp_var > 2)
    mooseError("GeneralizedPlaneStrainOffDiag only works for 1D axisymmetric or 2D Cartesian "
               "generalized plane strain cases!");

  for (unsigned int i = 0; i < _num_disp_var; ++i)
    _disp_var.push_back(&_subproblem.getVariable(_tid, nl_vnames[i]));

  for (unsigned int i = 0; i < _deigenstrain_dT.size(); ++i)
    _deigenstrain_dT[i] = &getMaterialPropertyDerivative<RankTwoTensor>(
        _base_name + _eigenstrain_names[i], _temp_var->name());

  if (isParamValid("scalar_variable_index_provider") &&
      !isParamValid("scalar_out_of_plane_strain_index"))
    mooseError("scalar_out_of_plane_strain_index should be provided if more "
               "than one is available");
}

void
GeneralizedPlaneStrainOffDiag::computeOffDiagJacobianScalar(unsigned int jvar)
{
  const unsigned int elem_scalar_var_id =
      _subblock_id_provider ? _subblock_id_provider->getSubblockIndex(*_current_elem) : 0;

  if (elem_scalar_var_id == _scalar_var_id)
  {
    if (_assembly.coordSystem() == Moose::COORD_XYZ)
      _scalar_out_of_plane_strain_direction = 2;
    else if (_assembly.coordSystem() == Moose::COORD_RZ)
      _scalar_out_of_plane_strain_direction = 1;

    if (_var.number() == _disp_var[0]->number())
      computeDispOffDiagJacobianScalar(0, jvar);
    else if (_num_disp_var == 2 && _var.number() == _disp_var[1]->number())
      computeDispOffDiagJacobianScalar(1, jvar);
    else if (isParamValid("temperature") ? _var.number() == _temp_var->number() : 0)
      computeTempOffDiagJacobianScalar(jvar);
  }
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
