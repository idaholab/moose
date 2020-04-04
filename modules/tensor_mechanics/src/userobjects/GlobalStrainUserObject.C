//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GlobalStrainUserObject.h"

#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", GlobalStrainUserObject);

InputParameters
GlobalStrainUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription(
      "Global Strain UserObject to provide Residual and diagonal Jacobian entry");
  params.addParam<std::vector<Real>>("applied_stress_tensor",
                                     "Vector of values defining the constant applied stress "
                                     "to add, in order 11, 22, 33, 23, 13, 12");
  params.addParam<std::string>("base_name", "Material properties base name");
  params.addCoupledVar("displacements", "The name of the displacement variables");
  params.set<ExecFlagEnum>("execute_on") = EXEC_LINEAR;

  return params;
}

GlobalStrainUserObject::GlobalStrainUserObject(const InputParameters & parameters)
  : ElementUserObject(parameters),
    GlobalStrainUserObjectInterface(),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _dstress_dstrain(getMaterialProperty<RankFourTensor>(_base_name + "Jacobian_mult")),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _dim(_mesh.dimension()),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp),
    _periodic_dir()
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var[i] = coupled("displacements", i);

  for (unsigned int dir = 0; dir < _dim; ++dir)
  {
    _periodic_dir(dir) = _mesh.isTranslatedPeriodic(_disp_var[0], dir);

    for (unsigned int i = 1; i < _ndisp; ++i)
      if (_mesh.isTranslatedPeriodic(_disp_var[i], dir) != _periodic_dir(dir))
        mooseError("All the displacement components in a particular direction should have same "
                   "periodicity.");
  }

  if (isParamValid("applied_stress_tensor"))
    _applied_stress_tensor.fillFromInputVector(
        getParam<std::vector<Real>>("applied_stress_tensor"));
  else
    _applied_stress_tensor.zero();
}

void
GlobalStrainUserObject::initialize()
{
  _residual.zero();
  _jacobian.zero();
}

void
GlobalStrainUserObject::execute()
{
  computeAdditionalStress();

  for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // residual, integral of stress components
    _residual += _JxW[_qp] * _coord[_qp] * (_stress[_qp] - _applied_stress_tensor);

    // diagonal jacobian, integral of elasticity tensor components
    _jacobian += _JxW[_qp] * _coord[_qp] * _dstress_dstrain[_qp];
  }
}

void
GlobalStrainUserObject::threadJoin(const UserObject & uo)
{
  const GlobalStrainUserObject & pstuo = static_cast<const GlobalStrainUserObject &>(uo);
  _residual += pstuo._residual;
  _jacobian += pstuo._jacobian;
}

void
GlobalStrainUserObject::finalize()
{
  std::vector<Real> residual(9);
  std::vector<Real> jacobian(81);

  std::copy(&_residual(0, 0), &_residual(0, 0) + 9, residual.begin());
  std::copy(&_jacobian(0, 0, 0, 0), &_jacobian(0, 0, 0, 0) + 81, jacobian.begin());

  gatherSum(residual);
  gatherSum(jacobian);

  std::copy(residual.begin(), residual.end(), &_residual(0, 0));
  std::copy(jacobian.begin(), jacobian.end(), &_jacobian(0, 0, 0, 0));
}

const RankTwoTensor &
GlobalStrainUserObject::getResidual() const
{
  return _residual;
}

const RankFourTensor &
GlobalStrainUserObject::getJacobian() const
{
  return _jacobian;
}

const VectorValue<bool> &
GlobalStrainUserObject::getPeriodicDirections() const
{
  return _periodic_dir;
}
