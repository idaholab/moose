//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressDivergenceTruss.h"

// MOOSE includes
#include "Assembly.h"
#include "Material.h"
#include "MooseVariable.h"
#include "SystemBase.h"
#include "RankTwoTensor.h"
#include "NonlinearSystem.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", StressDivergenceTruss);

InputParameters
StressDivergenceTruss::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Quasi-static and dynamic stress divergence kernel for truss element");
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction "
                                        "the variable this kernel acts in. (0 for disp_x, "
                                        "1 for disp_y, 2 for disp_z)");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

StressDivergenceTruss::StressDivergenceTruss(const InputParameters & parameters)
  : Kernel(parameters),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp),
    _force(getMaterialPropertyByName<Real>("forces")),
    _e_over_l(getMaterialPropertyByName<Real>("e_over_l")),
    _orientation(nullptr)
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var[i] = coupled("displacements", i);
}

void
StressDivergenceTruss::initialSetup()
{
  // setting up an orientation which is very similar to a rotation matrix
  _orientation = &_subproblem.assembly(_tid).getFE(FEType(), 1)->get_dxyzdxi();
}

void
StressDivergenceTruss::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  mooseAssert(_local_re.size() == 2, "Truss element must have two nodes only.");

  RealGradient orientation((*_orientation)[0]);
  orientation /= orientation.norm();

  VectorValue<Real> force_local = _force[0] * orientation;

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
StressDivergenceTruss::computeStiffness(unsigned int i, unsigned int j)
{
  RealGradient orientation((*_orientation)[0]);
  orientation /= orientation.norm();
  return orientation(i) * orientation(j) * _e_over_l[0];
}

void
StressDivergenceTruss::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());
  for (unsigned int i = 0; i < _test.size(); ++i)
    for (unsigned int j = 0; j < _phi.size(); ++j)
      if (_component < 3)
        _local_ke(i, j) = (i == j ? 1 : -1) * computeStiffness(_component, _component);

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
StressDivergenceTruss::computeOffDiagJacobian(unsigned int jvar_num)
{
  // auto jvar_num = jvar.number();
  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    // This (undisplaced) jvar could potentially yield the wrong phi size if this object is acting
    // on the displaced mesh
    auto phi_size = _sys.getVariable(_tid, jvar_num).dofIndices().size();

    for (unsigned int k = 0; k < _ndisp; ++k)
      if (jvar_num == _disp_var[k])
      {
        prepareMatrixTag(_assembly, _var.number(), jvar_num);
        for (unsigned int i = 0; i < _test.size(); ++i)
          for (unsigned int j = 0; j < phi_size; ++j)
            _local_ke(i, j) += (i == j ? 1 : -1) * computeStiffness(_component, k);
        accumulateTaggedLocalMatrix();
        return;
      }
    // Need some code here for coupling with temperature
  }
}
