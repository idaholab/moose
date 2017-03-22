/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StressDivergenceTensorsTruss.h"

#include "Assembly.h"
#include "Material.h"
#include "SystemBase.h"

template <>
InputParameters
validParams<StressDivergenceTensorsTruss>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Kernel for truss element");
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction "
                                        "the variable this kernel acts in. (0 for x, "
                                        "1 for y, 2 for z)");
  params.addCoupledVar("displacements",
                       "The string of displacements suitable for the problem statement");
  params.addCoupledVar("temp", "The temperature"); // Deprecated
  params.addCoupledVar("temperature", "The temperature");
  params.addCoupledVar("area", "Cross-sectional area of truss element");
  params.addParam<std::string>("base_name", "Material property base name");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

StressDivergenceTensorsTruss::StressDivergenceTensorsTruss(const InputParameters & parameters)
  : Kernel(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _axial_stress(getMaterialPropertyByName<Real>(_base_name + "axial_stress")),
    _e_over_l(getMaterialPropertyByName<Real>(_base_name + "e_over_l")),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _temp_coupled(isCoupled("temp") || isCoupled("temperature")),
    _temp_var(_temp_coupled ? (isCoupled("temp") ? coupled("temp") : coupled("temperature")) : 0),
    _area(coupledValue("area")),
    _orientation(NULL)
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var.push_back(coupled("displacements", i));

  // deprecate temp in favor of temperature
  if (isCoupled("temp"))
    mooseDeprecated("Use 'temperature' instead of 'temp'");
}

void
StressDivergenceTensorsTruss::initialSetup()
{
  _orientation = &_subproblem.assembly(_tid).getFE(FEType(), 1)->get_dxyzdxi();
}

void
StressDivergenceTensorsTruss::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  mooseAssert(re.size() == 2, "Truss element has and only has two nodes.");
  _local_re.resize(re.size());
  _local_re.zero();

  RealGradient orientation((*_orientation)[0]);
  orientation /= orientation.norm();

  VectorValue<Real> force_local = _axial_stress[0] * _area[0] * orientation;

  _local_re(0) = -force_local(_component);
  _local_re(1) = -_local_re(0);

  re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); ++i)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

Real
StressDivergenceTensorsTruss::computeStiffness(unsigned int i, unsigned int j)
{
  RealGradient orientation((*_orientation)[0]);
  orientation /= orientation.norm();

  return orientation(i) * orientation(j) * _e_over_l[0] * _area[0];
}

void
StressDivergenceTensorsTruss::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  for (unsigned int i = 0; i < _test.size(); ++i)
    for (unsigned int j = 0; j < _phi.size(); ++j)
      _local_ke(i, j) += (i == j ? 1 : -1) * computeStiffness(_component, _component);

  ke += _local_ke;

  if (_has_diag_save_in)
  {
    unsigned int rows = ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; ++i)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _diag_save_in.size(); ++i)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

void
StressDivergenceTensorsTruss::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
  else
  {
    unsigned int coupled_component = 0;
    bool disp_coupled = false;

    for (unsigned int i = 0; i < _ndisp; ++i)
      if (jvar == _disp_var[i])
      {
        coupled_component = i;
        disp_coupled = true;
        break;
      }

    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);

    if (disp_coupled)
      for (unsigned int i = 0; i < _test.size(); ++i)
        for (unsigned int j = 0; j < _phi.size(); ++j)
          ke(i, j) += (i == j ? 1 : -1) * computeStiffness(_component, coupled_component);
    else if (false) // Need some code here for coupling with temperature
    {
    }
  }
}
