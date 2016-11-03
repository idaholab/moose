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

template<>
InputParameters validParams<GeneralizedPlaneStrainOffDiag>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Generalized Plane Strain kernel to provide contribution of the out-of-plane strain to other kernels");
  params.addRequiredParam<std::vector<NonlinearVariableName> >("displacements", "Variable for the displacements");
  params.addParam<NonlinearVariableName>("temperature", "Variable for the temperature");
  params.addCoupledVar("scalar_strain_zz", "Scalar variable for the strain_zz");
  params.addParam<std::string>("base_name", "Material property base name");
  params.set<bool>("use_displaced_mesh") = true;

  return params;
}

GeneralizedPlaneStrainOffDiag::GeneralizedPlaneStrainOffDiag(const InputParameters & parameters) :
   DerivativeMaterialInterface<Kernel>(parameters),
   _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
   _Jacobian_mult(getMaterialProperty<RankFourTensor>(_base_name + "Jacobian_mult")),
   _thermal_expansion_tensor(getDefaultMaterialProperty<RankTwoTensor>(_base_name + "_thermal_expansion_tensor")),
   _scalar_strain_zz_var(coupledScalar("scalar_strain_zz")),
   _temp_var(isParamValid("temperature") ? &_fe_problem.getVariable(_tid, getParam<NonlinearVariableName>("temperature")) : NULL)
{
  const std::vector<NonlinearVariableName> & nl_vnames(getParam<std::vector<NonlinearVariableName> >("displacements"));
  if (nl_vnames.size() != 2)
    mooseError("GeneralizedPlaneStrain only works for two dimensional case!");

  for (unsigned int i = 0; i < nl_vnames.size(); ++i)
    _disp_var.push_back(&_fe_problem.getVariable(_tid, nl_vnames[i]));
}

void
GeneralizedPlaneStrainOffDiag::computeOffDiagJacobianScalar(unsigned int jvar)
{
  if (_var.number() == _disp_var[0]->number())
    computeDispOffDiagJacobianScalar(0, jvar);
  else if (_var.number() == _disp_var[1]->number())
    computeDispOffDiagJacobianScalar(1, jvar);
  else if (isParamValid("temperature") ? _var.number() == _temp_var->number() : 0)
    computeTempOffDiagJacobianScalar(jvar);
}

void
GeneralizedPlaneStrainOffDiag::computeDispOffDiagJacobianScalar(unsigned int component, unsigned int jvar)
{
  if (jvar == _scalar_strain_zz_var)
  {
    DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), jvar);
    DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar, _var.number());
    MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);

    for (_i = 0; _i < _test.size(); ++_i)
      for (_j = 0; _j < jv.order(); ++_j)
        for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
        {
          ken(_i, _j) += _JxW[_qp] * _coord[_qp] * _Jacobian_mult[_qp](2, 2, component, component) * _grad_test[_i][_qp](component);
          kne(_j, _i) += _JxW[_qp] * _coord[_qp] * _Jacobian_mult[_qp](2, 2, component, component) * _grad_test[_i][_qp](component);
        }
  }
}

void
GeneralizedPlaneStrainOffDiag::computeTempOffDiagJacobianScalar(unsigned int jvar)
{
  if (jvar == _scalar_strain_zz_var)
  {
    DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar, _var.number());
    MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);

    for (_i = 0; _i < _test.size(); ++_i)
      for (_j = 0; _j < jv.order(); ++_j)
        for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
          kne(_j, _i) += _JxW[_qp] * _coord[_qp] * (_Jacobian_mult[_qp] * _thermal_expansion_tensor[_qp])(2, 2) * _test[_i][_qp];
  }
}
