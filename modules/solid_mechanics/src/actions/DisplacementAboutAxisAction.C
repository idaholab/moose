/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DisplacementAboutAxisAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "DisplacementAboutAxis.h"

template<>
InputParameters validParams<DisplacementAboutAxisAction>()
{
  InputParameters params = validParams<Action>();
  addDisplacementAboutAxisParams(params);
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where the rotational displacement will be applied");
  params.addRequiredParam<NonlinearVariableName>("disp_x", "The x displacement");
  params.addParam<NonlinearVariableName>("disp_y", "", "The y displacement");
  params.addParam<NonlinearVariableName>("disp_z", "", "The z displacement");

  params.addParam<std::vector<AuxVariableName> >("save_in_disp_x", "The save_in variables for x displacement");
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_y", "The save_in variables for y displacement");
  params.addParam<std::vector<AuxVariableName> >("save_in_disp_z", "The save_in variables for z displacement");

  params.addParam<bool>("constrain_axial_motion",true,"Whether to constrain axial motion (can only be false if the axis of rotation is parallel to one of the cartesian axes)");
  return params;
}

DisplacementAboutAxisAction::DisplacementAboutAxisAction(const InputParameters & params) :
  Action(params),
  _boundary(getParam<std::vector<BoundaryName> >("boundary")),
  _disp_x(getParam<NonlinearVariableName>("disp_x")),
  _disp_y(getParam<NonlinearVariableName>("disp_y")),
  _disp_z(getParam<NonlinearVariableName>("disp_z")),
  _axis_origin(getParam<RealVectorValue>("axis_origin")),
  _axis_direction(getParam<RealVectorValue>("axis_direction")),
  _constrain_axial_motion(getParam<bool>("constrain_axial_motion")),
  _kernel_name("DisplacementAboutAxis"),
  _use_displaced_mesh(true)
{
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName> >("save_in_disp_x"));
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName> >("save_in_disp_y"));
  _save_in_vars.push_back(getParam<std::vector<AuxVariableName> >("save_in_disp_z"));

  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_x"));
  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_y"));
  _has_save_in_vars.push_back(params.isParamValid("save_in_disp_z"));
}

void
DisplacementAboutAxisAction::act()
{
  // Check whether to constrain the axial direction and build vector of the dimensions to constrain
  _axis_direction = _axis_direction.unit();
  std::vector<unsigned int> dim_vec;
  if (_constrain_axial_motion == false)
  {
    RealVectorValue xaxis(1,0,0);
    RealVectorValue yaxis(0,1,0);
    RealVectorValue zaxis(0,0,1);
    if (_axis_direction.absolute_fuzzy_equals(xaxis,1.e-15) || _axis_direction.absolute_fuzzy_equals(-1*xaxis,1.e-15))
    {
      dim_vec.push_back(1);
      dim_vec.push_back(2);
    }
    else if (_axis_direction.absolute_fuzzy_equals(yaxis,1.e-15) || _axis_direction.absolute_fuzzy_equals(-1*yaxis,1.e-15))
    {
      dim_vec.push_back(0);
      dim_vec.push_back(2);
    }
    else if (_axis_direction.absolute_fuzzy_equals(zaxis,1.e-15) || _axis_direction.absolute_fuzzy_equals(-1*zaxis,1.e-15))
    {
      dim_vec.push_back(0);
      dim_vec.push_back(1);
    }
    else
      mooseError("In DisplacementAboutAxisAction, constrain_axial_motion = false but the axis of rotation is not parallel to one of the Cartesian axes.");
  }
  else
  {
    dim_vec.push_back(0);
    dim_vec.push_back(1);
    dim_vec.push_back(2);
  }

  std::vector<NonlinearVariableName> vars;
  vars.push_back(_disp_x);
  vars.push_back(_disp_y);
  vars.push_back(_disp_z);
  for (std::vector<unsigned int>::iterator it = dim_vec.begin(); it != dim_vec.end(); ++it)
  {
    std::stringstream name;
    name << _name;
    name << "_";
    name << *it;

    InputParameters params = _factory.getValidParams(_kernel_name);

    params.set<std::vector<BoundaryName> >("boundary") = _boundary;
    params.set<FunctionName>("function") = getParam<FunctionName>("function");
    params.set<MooseEnum>("angle_units") = getParam<MooseEnum>("angle_units");
    params.set<RealVectorValue>("axis_origin") = getParam<RealVectorValue>("axis_origin");
    params.set<RealVectorValue>("axis_direction") = getParam<RealVectorValue>("axis_direction");

    params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

    params.set<int>("component") = *it;
    params.set<NonlinearVariableName>("variable") = vars[*it];
    if (_has_save_in_vars[*it])
      params.set<std::vector<AuxVariableName> >("save_in") = _save_in_vars[*it];

    _problem->addBoundaryCondition(_kernel_name, name.str(), params);
  }

}
