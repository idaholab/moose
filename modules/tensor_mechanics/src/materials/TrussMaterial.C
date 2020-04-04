//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TrussMaterial.h"

// MOOSE includes
#include "Material.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "NonlinearSystem.h"

#include "libmesh/quadrature.h"

InputParameters
TrussMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addRequiredParam<std::vector<VariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addCoupledVar("youngs_modulus", "Variable containing Young's modulus");
  return params;
}

TrussMaterial::TrussMaterial(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _youngs_modulus(coupledValue("youngs_modulus")),
    _total_stretch(declareProperty<Real>(_base_name + "total_stretch")),
    _elastic_stretch(declareProperty<Real>(_base_name + "elastic_stretch")),
    _axial_stress(declareProperty<Real>(_base_name + "axial_stress")),
    _e_over_l(declareProperty<Real>(_base_name + "e_over_l"))
{
  const std::vector<VariableName> & nl_vnames(getParam<std::vector<VariableName>>("displacements"));
  _ndisp = nl_vnames.size();

  // fetch nonlinear variables
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var.push_back(&_fe_problem.getStandardVariable(_tid, nl_vnames[i]));
}

void
TrussMaterial::initQpStatefulProperties()
{
  _axial_stress[_qp] = 0.0;
  _total_stretch[_qp] = 0.0;
  _elastic_stretch[_qp] = 0.0;
}

void
TrussMaterial::computeProperties()
{
  // check for consistency of the number of element nodes
  mooseAssert(_current_elem->n_nodes() == 2, "Truss element needs to have exactly two nodes.");

  // fetch the two end nodes for _current_elem
  std::vector<const Node *> node;
  for (unsigned int i = 0; i < 2; ++i)
    node.push_back(_current_elem->node_ptr(i));

  // calculate original length of a truss element
  RealGradient dxyz;
  for (unsigned int i = 0; i < _ndisp; ++i)
    dxyz(i) = (*node[1])(i) - (*node[0])(i);
  _origin_length = dxyz.norm();

  // fetch the solution for the two end nodes
  NonlinearSystemBase & nonlinear_sys = _fe_problem.getNonlinearSystemBase();
  const NumericVector<Number> & sol = *nonlinear_sys.currentSolution();

  std::vector<Real> disp0, disp1;
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    disp0.push_back(sol(node[0]->dof_number(nonlinear_sys.number(), _disp_var[i]->number(), 0)));
    disp1.push_back(sol(node[1]->dof_number(nonlinear_sys.number(), _disp_var[i]->number(), 0)));
  }

  // calculate current length of a truss element
  for (unsigned int i = 0; i < _ndisp; ++i)
    dxyz(i) += disp1[i] - disp0[i];
  _current_length = dxyz.norm();

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _e_over_l[_qp] = _youngs_modulus[_qp] / _origin_length;

    computeQpStrain();
    computeQpStress();
  }
}
