//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressDivergenceSpring.h"

#include "Assembly.h"
#include "MooseVariable.h"
#include "SystemBase.h"

registerMooseObject("SolidMechanicsApp", StressDivergenceSpring);

InputParameters
StressDivergenceSpring::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Internal force of a two-node spring element on one of its six "
                             "degrees of freedom, from the force of a spring law material.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component",
      "component < 6",
      "The degree of freedom this kernel acts on: 0, 1, 2 for the displacements along the "
      "global X, Y, Z axes, 3, 4, 5 for the rotations about them. The variable of the kernel "
      "must be the matching entry of displacements or rotations.");
  params.addRequiredCoupledVar("displacements",
                               "The three displacement variables, in the order x, y, z, for every "
                               "component.");
  params.addParam<std::string>("base_name",
                               "The base_name of the deformation material and the law of this "
                               "spring, when several springs share a block.");
  params.addCoupledVar("rotations",
                       "The three rotation variables, in the order x, y, z, needed for the "
                       "components 3 to 5.");
  return params;
}

StressDivergenceSpring::StressDivergenceSpring(const InputParameters & parameters)
  : Kernel(parameters),
    _component(getParam<unsigned int>("component")),
    _axis(_component % 3),
    _rotational(_component >= 3),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _rotation_matrix(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "spring_rotation_matrix")),
    _force(getMaterialPropertyByName<RealVectorValue>(
        _base_name + (_rotational ? "spring_moment" : "spring_force"))),
    _stiffness(getMaterialPropertyByName<RankTwoTensor>(
        _base_name + (_rotational ? "spring_rotational_stiffness" : "spring_stiffness")))
{
  const std::string group = _rotational ? "rotations" : "displacements";
  if (coupledComponents(group) != 3)
    paramError(group, "Three variables are needed, one per global axis.");
  for (unsigned int i = 0; i < 3; ++i)
    _group_vars.push_back(coupled(group, i));
  if (_group_vars[_axis] != _var.number())
    paramError("variable",
               "The variable must be entry ",
               _axis,
               " of '",
               group,
               "' for component ",
               _component,
               ".");
}

RankTwoTensor
StressDivergenceSpring::globalStiffness() const
{
  return _rotation_matrix[0].transpose() * _stiffness[0] * _rotation_matrix[0];
}

void
StressDivergenceSpring::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());
  mooseAssert(_local_re.size() == 2, "A spring element has two nodes.");

  const RealVectorValue global_force = _rotation_matrix[0].transpose() * _force[0];
  _local_re(0) = -global_force(_axis);
  _local_re(1) = global_force(_axis);

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); ++i)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

void
StressDivergenceSpring::fillLocalMatrix(const RankTwoTensor & global_stiffness,
                                        unsigned int coupled_axis,
                                        unsigned int phi_size)
{
  const Real entry = global_stiffness(_axis, coupled_axis);
  for (unsigned int i = 0; i < _test.size(); ++i)
    for (unsigned int j = 0; j < phi_size; ++j)
      _local_ke(i, j) += (i == j ? 1.0 : -1.0) * entry;
}

void
StressDivergenceSpring::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());
  fillLocalMatrix(globalStiffness(), _axis, _phi.size());
  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in)
  {
    const unsigned int rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; ++i)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _diag_save_in.size(); ++i)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

void
StressDivergenceSpring::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (jvar_num == _var.number())
  {
    computeJacobian();
    return;
  }

  // The laws couple the three translations among themselves and the three rotations among
  // themselves, never a translation with a rotation.
  for (unsigned int coupled_axis = 0; coupled_axis < 3; ++coupled_axis)
    if (jvar_num == _group_vars[coupled_axis])
    {
      const auto & jvar = getVariable(jvar_num);
      prepareMatrixTag(_assembly, _var.number(), jvar_num);
      fillLocalMatrix(globalStiffness(), coupled_axis, jvar.dofIndices().size());
      accumulateTaggedLocalMatrix();
      return;
    }
}
