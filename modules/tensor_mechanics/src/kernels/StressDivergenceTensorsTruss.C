//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressDivergenceTensorsTruss.h"

// MOOSE includes
#include "Assembly.h"
#include "Material.h"
#include "MooseVariable.h"
#include "SystemBase.h"

registerMooseObject("TensorMechanicsApp", StressDivergenceTensorsTruss);

InputParameters
StressDivergenceTensorsTruss::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Kernel for truss element");
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction "
                                        "the variable this kernel acts in. (0 for x, "
                                        "1 for y, 2 for z)");
  params.addCoupledVar("displacements",
                       "The string of displacements suitable for the problem statement");
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
    _temp_coupled(isCoupled("temperature")),
    _temp_var(_temp_coupled ? coupled("temperature") : 0),
    _area(coupledValue("area")),
    _orientation(NULL)
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var.push_back(coupled("displacements", i));
}

void
StressDivergenceTensorsTruss::initialSetup()
{
  _orientation = &_subproblem.assembly(_tid).getFE(FEType(), 1)->get_dxyzdxi();
}

void
StressDivergenceTensorsTruss::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  mooseAssert(_local_re.size() == 2, "Truss element has and only has two nodes.");

  RealGradient orientation((*_orientation)[0]);
  orientation /= orientation.norm();

  VectorValue<Real> force_local = _axial_stress[0] * _area[0] * orientation;

  _local_re(0) = -force_local(_component);
  _local_re(1) = -_local_re(0);

  accumulateTaggedLocalResidual();

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
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  for (unsigned int i = 0; i < _test.size(); ++i)
    for (unsigned int j = 0; j < _phi.size(); ++j)
      _local_ke(i, j) += (i == j ? 1 : -1) * computeStiffness(_component, _component);

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in)
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; ++i)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _diag_save_in.size(); ++i)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

void
StressDivergenceTensorsTruss::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    const auto & jvar = getVariable(jvar_num);

    // This (undisplaced) jvar could potentially yield the wrong phi size if this object is acting
    // on the displaced mesh
    auto phi_size = jvar.dofIndices().size();

    unsigned int coupled_component = 0;
    bool disp_coupled = false;

    for (unsigned int i = 0; i < _ndisp; ++i)
      if (jvar_num == _disp_var[i])
      {
        coupled_component = i;
        disp_coupled = true;
        break;
      }

    if (disp_coupled)
    {
      prepareMatrixTag(_assembly, _var.number(), jvar_num);

      for (unsigned int i = 0; i < _test.size(); ++i)
        for (unsigned int j = 0; j < phi_size; ++j)
          _local_ke(i, j) += (i == j ? 1 : -1) * computeStiffness(_component, coupled_component);

      accumulateTaggedLocalMatrix();
    }
    else if (false) // Need some code here for coupling with temperature
    {
    }
  }
}
