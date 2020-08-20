//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeIncrementalTrussStrain.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "NonlinearSystem.h"
#include "MooseVariable.h"
#include "Function.h"

#include "libmesh/quadrature.h"
#include "libmesh/utility.h"

#include "Material.h"
#include "MooseVariable.h"

registerMooseObject("TensorMechanicsApp", ComputeIncrementalTrussStrain);

InputParameters
ComputeIncrementalTrussStrain::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute a infinitesimal/large strain increment for the truss.");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addCoupledVar("youngs_modulus", "Variable containing Young's modulus");
  params.addRequiredCoupledVar(
      "area",
      "Cross-section area of the truss. Can be supplied as either a number or a variable name.");
  params.addParam<bool>("large_strain", false, "Set to true if large strain are to be calculated.");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", "List of truss eigenstrains to be applied in this strain calculation.");
  return params;
}

ComputeIncrementalTrussStrain::ComputeIncrementalTrussStrain(const InputParameters & parameters)
  : Material(parameters),
    _ndisp(coupledComponents("displacements")),
    _disp_num(_ndisp),
    _area(coupledValue("area")),
    _original_length(declareProperty<Real>("original_length")),
    _current_length(declareProperty<Real>("current_length")),
    _total_disp_strain(declareProperty<RealVectorValue>("total_disp_strain")),
    _total_disp_strain_old(getMaterialPropertyOld<RealVectorValue>("total_disp_strain")),
    _mech_disp_strain_increment(declareProperty<RealVectorValue>("mech_disp_strain_increment")),
    _material_stiffness(getMaterialPropertyByName<Real>("material_stiffness")),
    _e_over_l(declareProperty<Real>("e_over_l")),
    _large_strain(getParam<bool>("large_strain")),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _disp_eigenstrain(_eigenstrain_names.size()),
    _disp_eigenstrain_old(_eigenstrain_names.size())
{
  const std::vector<VariableName> & nl_vnames(getParam<std::vector<VariableName>>("displacements"));
  // fetch coupled variables and gradients (as stateful properties if necessary)
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var.push_back(&_fe_problem.getStandardVariable(_tid, nl_vnames[i]));

  // fetch coupled variables and gradients (as stateful properties if necessary)
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    MooseVariable * disp_variable = getVar("displacements", i);
    _disp_num[i] = disp_variable->number();
  }

  if (_large_strain)
    mooseError("ComputeIncrementalTrussStrain: Large strain calculation does not currently "
               "support asymmetric truss configurations with non-zero first or third moments of "
               "area.");

  for (unsigned int i = 0; i < _eigenstrain_names.size(); ++i)
  {
    _disp_eigenstrain[i] = &getMaterialProperty<RealVectorValue>("disp_" + _eigenstrain_names[i]);
    _disp_eigenstrain_old[i] =
        &getMaterialPropertyOld<RealVectorValue>("disp_" + _eigenstrain_names[i]);
  }
}

void
ComputeIncrementalTrussStrain::initQpStatefulProperties()
{
  RealVectorValue temp;
  _total_disp_strain[_qp] = temp;
}

void
ComputeIncrementalTrussStrain::computeProperties()
{
  // check for consistency of the number of element nodes
  mooseAssert(_current_elem->n_nodes() == 2, "Truss element needs to have exactly two nodes.");

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp){
    out << "_qp " << _qp << " _qrule " << _qrule  << std::endl;
    computeQpStrain();
  }

  if (_fe_problem.currentlyComputingJacobian())
    computeStiffnessMatrix();
}

void
ComputeIncrementalTrussStrain::computeQpStrain()
{
  out << "computeQpStrain qp "<< _qp << std::endl;

  std::vector<const Node *> node;
  for (unsigned int i = 0; i < 2; ++i)
    node.push_back(_current_elem->node_ptr(i));
  // calculate original length of a truss element
  // Nodal positions do not change with time as undisplaced mesh is used by material classes by
  // default
  RealGradient dxyz;
  for (unsigned int i = 0; i < _ndisp; ++i)
    dxyz(i) = (*node[1])(i) - (*node[0])(i);
  _original_length[_qp] = dxyz.norm();

  // fetch the solution for the two end nodes to calculate the current length of a truss element
  NonlinearSystemBase & nonlinear_sys = _fe_problem.getNonlinearSystemBase();
  const NumericVector<Number> & sol = *nonlinear_sys.currentSolution();
  std::vector<Real> disp0, disp1;
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    disp0.push_back(sol(node[0]->dof_number(nonlinear_sys.number(), _disp_var[i]->number(), 0)));
    disp1.push_back(sol(node[1]->dof_number(nonlinear_sys.number(), _disp_var[i]->number(), 0)));
  }
  for (unsigned int i = 0; i < _ndisp; ++i)
    dxyz(i) += disp1[i] - disp0[i];
  _current_length[_qp] = dxyz.norm();

  _mech_disp_strain_increment[_qp](0) = (_current_length[_qp] / _original_length[_qp] - 1.0)* _area[_qp];
  _mech_disp_strain_increment[_qp](1) = 0.;
  _mech_disp_strain_increment[_qp](2) = 0.;

  _total_disp_strain[_qp] = _mech_disp_strain_increment[_qp];
  for (unsigned int i = 0; i < _eigenstrain_names.size(); ++i)
  {
    _mech_disp_strain_increment[_qp] -=
        ((*_disp_eigenstrain[i])[_qp] - (*_disp_eigenstrain_old[i])[_qp]) * _area[_qp];
    // _mech_disp_strain_increment[_qp] -=
    //     _total_rotation[0] * ((*_disp_eigenstrain[i])[_qp] - (*_disp_eigenstrain_old[i])[_qp]) *
    //     _area[_qp];
  }
}

void
ComputeIncrementalTrussStrain::computeStiffnessMatrix()
{
  out << "computeStiffnessMatrix qp "<< _qp << std::endl;

  std::vector<const Node *> node;
  for (unsigned int i = 0; i < 2; ++i)
    node.push_back(_current_elem->node_ptr(i));
  // calculate original length of a truss element
  // Nodal positions do not change with time as undisplaced mesh is used by material classes by
  // default
  RealGradient dxyz;
  for (unsigned int i = 0; i < _ndisp; ++i)
    dxyz(i) = (*node[1])(i) - (*node[0])(i);
  _original_length[_qp] = dxyz.norm();

  Real _e_over_l_local = _material_stiffness[_qp] * _area[_qp]  / _original_length[_qp];
  _e_over_l[_qp] = _e_over_l_local;
}
