/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "TrussMaterial.h"
#include "Material.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"

//libmesh includes
#include "libmesh/quadrature.h" 

template<>
InputParameters validParams<TrussMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define multiple mechanics material systems on the same block, i.e. for multiple phases");
  params.addRequiredParam<std::vector<NonlinearVariableName> >("displacements", "The displacements appropriate for the simulation geometry and coordinate system");
  params.addRequiredParam<Real>("youngs_modulus", "Young's modulus for truss element");
  params.addCoupledVar("youngs_modulus_var","Variable containing Young's modulus");
  return params;
}

TrussMaterial::TrussMaterial(const InputParameters & parameters) :
    Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _my_youngs_modulus(isParamValid("youngs_modulus") ? getParam<Real>("youngs_modulus") : 0),
    _youngs_modulus_coupled(isCoupled("youngs_modulus_var")),
    _youngs_modulus_var(_youngs_modulus_coupled ? coupledValue("youngs_modulus_var"): _zero),  
    _total_stretch(declareProperty<Real>(_base_name + "total_stretch")),
    _elastic_stretch(declareProperty<Real>(_base_name + "elastic_stretch")),
    _axial_stress(declareProperty<Real>(_base_name + "axial_stress")),
    _e_over_l(declareProperty<Real>(_base_name + "e_over_l"))
{
  const std::vector<NonlinearVariableName> & nl_vnames(getParam<std::vector<NonlinearVariableName> >("displacements"));
  _ndisp = nl_vnames.size();

  // fetch nonlinear variables
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var.push_back(&_fe_problem.getVariable(_tid, nl_vnames[i]));

  if (parameters.isParamValid("youngs_modulus"))
  {
    if (_youngs_modulus_coupled)
      mooseError("Cannot specify both youngs_modulus and youngs_modulus_var");
  }
  else
  {
    if (!_youngs_modulus_coupled)
      mooseError("Must specify either youngs_modulus or youngs_modulus_var");
  }
}

 void
TrussMaterial::initQpStatefulProperties()
{
  _axial_stress[_qp] = 0;
  _total_stretch[_qp] = 0;
  _elastic_stretch[_qp] = 0;
}

void
TrussMaterial::computeProperties()
{
  // check for consistency of the number of element nodes
  mooseAssert(_current_elem->n_nodes() == 2, "Truss element has and only has two nodes.");

  // fetch the two end nodes for _current_elem
  std::vector<Node*> node;
  for (unsigned int i = 0; i < 2; ++i)
    node.push_back(_current_elem->get_node(i));

  // calculate original length of a truss element
  RealGradient dxyz;
  for (unsigned int i = 0; i < _ndisp; ++i)
    dxyz(i) = (*node[1])(i) - (*node[0])(i);
  _origin_length = dxyz.norm();

  // fetch the solution for the two end nodes
  NonlinearSystem& nonlinear_sys = _fe_problem.getNonlinearSystem();
  const NumericVector<Number> &sol = *nonlinear_sys.currentSolution();

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
    _youngs_modulus = _youngs_modulus_coupled ? _youngs_modulus_var[_qp] : _my_youngs_modulus;
    _e_over_l[_qp] = _youngs_modulus / _origin_length;

    computeQpStrain();
    computeQpStress();
  }
}
