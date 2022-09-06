//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledHeatTransferAction.h"
#include "DiscreteLineSegmentInterface.h"
#include "FEProblem.h"

registerMooseAction("ThermalHydraulicsApp", CoupledHeatTransferAction, "add_bc");
registerMooseAction("ThermalHydraulicsApp", CoupledHeatTransferAction, "add_user_object");
registerMooseAction("ThermalHydraulicsApp", CoupledHeatTransferAction, "add_transfer");

InputParameters
CoupledHeatTransferAction::validParams()
{
  InputParameters params = Action::validParams();

  params.addClassDescription(
      "Action that creates the necessary objects, for the solid side, to couple a "
      "solid heat conduction region to a 1-D flow channel via convective heat transfer");

  params.addRequiredParam<std::vector<BoundaryName>>("boundary",
                                                     "Boundary name(s) on the solid side");
  params.addRequiredParam<VariableName>("T", "Solid side temperature variable");
  params.addRequiredParam<VariableName>(
      "T_wall", "Variable on the flow channel side into which to transfer the solid temperature");
  params.addRequiredParam<std::vector<VariableName>>(
      "T_fluid", "Variable(s) on the solid side into which to transfer the fluid temperature(s)");
  params.addRequiredParam<std::vector<VariableName>>(
      "htc",
      "Variable(s) on the solid side into which to transfer the heat transfer coefficient(s)");
  params.addParam<std::vector<VariableName>>(
      "kappa", "Variables on the solid side into which to transfer the wall contact fractions");
  params.addParam<std::vector<UserObjectName>>(
      "T_fluid_user_objects", "Spatial user object(s) holding the fluid temperature values");
  params.addParam<std::vector<UserObjectName>>(
      "htc_user_objects", "Spatial user object(s) holding the heat transfer coefficient values");
  params.addDeprecatedParam<UserObjectName>(
      "T_fluid_user_object",
      "Spatial user object holding the fluid temperature values",
      "This parameter is deprecated in favor of 'T_fluid_user_objects' (just add an 's' to "
      "parameter name).");
  params.addDeprecatedParam<UserObjectName>(
      "htc_user_object",
      "Spatial user object holding the heat transfer coefficient values",
      "This parameter is deprecated in favor of 'htc_user_objects' (just add an 's' to parameter "
      "name).");
  params.addParam<std::vector<UserObjectName>>(
      "kappa_user_objects", "Spatial user object(s) holding the wall contact fraction values");

  MooseEnum directions("x y z");
  params.addDeprecatedParam<MooseEnum>(
      "direction",
      directions,
      "The direction of the layers.",
      "The usage of 'direction' and 'num_layers' is deprecated. Use 'position', 'orientation', "
      "'rotation', 'length', and 'n_elems' instead. The latter parameters correspond to the "
      "parameters of the same names in the coupled flow channel component.");
  params.addDeprecatedParam<unsigned int>(
      "num_layers",
      "The number of layers.",
      "The usage of 'direction' and 'num_layers' is deprecated. Use 'position', 'orientation', "
      "'rotation', 'length', and 'n_elems' instead. The latter parameters correspond to the "
      "parameters of the same names in the coupled flow channel component.");

  params.addParam<Point>("position", "Start position of axis in 3-D space [m]");
  params.addParam<RealVectorValue>(
      "orientation",
      "Direction of axis from start position to end position (no need to normalize)");
  params.addParam<Real>("rotation", 0.0, "Angle of rotation about the x-axis [degrees]");
  params.addParam<std::vector<Real>>("length", "Length of each axial section [m]");
  params.addParam<std::vector<unsigned int>>("n_elems", "Number of elements in each axial section");

  params.addRequiredParam<std::string>("multi_app", "The name of the multi-app.");

  params.addParam<std::vector<Point>>(
      "positions", "Sub-app positions. Each set of 3 values represents a Point.");
  params.addParam<FileName>(
      "positions_file",
      "Name of file containing sub-app positions. Each set of 3 values represents a Point.");
  MultiAppTransfer::addSkipCoordCollapsingParam(params);
  return params;
}

CoupledHeatTransferAction::CoupledHeatTransferAction(const InputParameters & params)
  : Action(params),

    _boundary(getParam<std::vector<BoundaryName>>("boundary")),

    _T_solid_var_name(getParam<VariableName>("T")),
    _T_wall_var_name(getParam<VariableName>("T_wall")),
    _T_fluid_var_names(getParam<std::vector<VariableName>>("T_fluid")),
    _htc_var_names(getParam<std::vector<VariableName>>("htc")),

    _n_phases(_T_fluid_var_names.size()),

    _T_wall_user_object_name(name() + "_T_avg_uo"),

    _multi_app_name(getParam<std::string>("multi_app"))
{
  if (isParamValid("T_fluid_user_objects"))
    _T_fluid_user_object_names = getParam<std::vector<UserObjectName>>("T_fluid_user_objects");
  else if (isParamValid("T_fluid_user_object"))
    _T_fluid_user_object_names = {getParam<UserObjectName>("T_fluid_user_object")};
  else
    mooseError("The parameter 'T_fluid_user_objects' must be specified.");

  if (isParamValid("htc_user_objects"))
    _htc_user_object_names = getParam<std::vector<UserObjectName>>("htc_user_objects");
  else if (isParamValid("htc_user_object"))
    _htc_user_object_names = {getParam<UserObjectName>("htc_user_object")};
  else
    mooseError("The parameter 'htc_user_objects' must be specified.");

  if (_htc_var_names.size() != _n_phases || _T_fluid_user_object_names.size() != _n_phases ||
      _htc_user_object_names.size() != _n_phases)
    mooseError("The parameters 'T_fluid', 'htc', 'T_fluid_user_objects', and 'htc_user_objects' "
               "must have the same numbers of elements.");

  if (_n_phases == 1)
  {
    if (isParamValid("kappa") || isParamValid("kappa_user_objects"))
      mooseError("If there is only one phase (e.g., only one element in 'T_fluid'), then the "
                 "parameters 'kappa' and 'kappa_user_objects' must not be provided.");
  }
  else
  {
    if (!isParamValid("kappa") || !isParamValid("kappa_user_objects"))
      mooseError("If there is more than one phase (e.g., more than one element in 'T_fluid'), then "
                 "the parameters 'kappa' and 'kappa_user_objects' must be provided.");
    else
    {
      _kappa_var_names = getParam<std::vector<VariableName>>("kappa");
      _kappa_user_object_names = getParam<std::vector<UserObjectName>>("kappa_user_objects");
      if (_kappa_var_names.size() != _n_phases || _kappa_user_object_names.size() != _n_phases)
        mooseError("The parameters 'kappa' and 'kappa_user_objects' must have the same number of "
                   "elements as 'T_fluid'.");
    }
  }

  if (isParamValid("orientation"))
  {
    const auto & orientation = getParam<RealVectorValue>("orientation");
    if (!DiscreteLineSegmentInterface::getAlignmentAxis(orientation).isValid())
      mooseError("The direction given by the parameter 'orientation' must be aligned with the x, "
                 "y, or z axis.");
  }
}

void
CoupledHeatTransferAction::act()
{
  if (_current_task == "add_bc")
    addBCs();
  else if (_current_task == "add_user_object")
    addUserObjects();
  else if (_current_task == "add_transfer")
    addTransfers();
}

void
CoupledHeatTransferAction::addBCs()
{
  for (unsigned int k = 0; k < _n_phases; k++)
  {
    const std::string class_name = "CoupledConvectiveHeatFluxBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = _T_solid_var_name;
    params.set<std::vector<BoundaryName>>("boundary") = {_boundary};
    params.set<std::vector<VariableName>>("T_infinity") = {_T_fluid_var_names[k]};
    params.set<std::vector<VariableName>>("htc") = {_htc_var_names[k]};
    if (_n_phases > 1)
      params.set<std::vector<VariableName>>("scale_factor") = {_kappa_var_names[k]};
    _problem->addBoundaryCondition(class_name, name() + "_bc" + std::to_string(k), params);
  }
}

void
CoupledHeatTransferAction::addUserObjects()
{
  // Solid temperature spatial user object
  {
    const std::string class_name = "NearestPointLayeredSideAverage";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<VariableName>>("variable") = {_T_solid_var_name};
    params.set<std::vector<BoundaryName>>("boundary") = {_boundary};

    // set sub-app positions
    if (isParamValid("positions"))
      params.set<std::vector<Point>>("points") = getParam<std::vector<Point>>("positions");
    else if (isParamValid("positions_file"))
      params.set<FileName>("points_file") = getParam<FileName>("positions_file");
    else
      params.set<std::vector<Point>>("points") = {Point(0, 0, 0)};

    // set layers
    if (isParamValid("direction") && isParamValid("num_layers"))
    {
      params.set<MooseEnum>("direction") = getParam<MooseEnum>("direction");
      params.set<unsigned int>("num_layers") = getParam<unsigned int>("num_layers");
    }
    else if (isParamValid("position") && isParamValid("orientation") && isParamValid("length") &&
             isParamValid("n_elems"))
    {
      const auto & position = getParam<Point>("position");
      const auto & orientation = getParam<RealVectorValue>("orientation");
      const auto & rotation = getParam<Real>("rotation");
      const auto & lengths = getParam<std::vector<Real>>("length");
      const auto & n_elems = getParam<std::vector<unsigned int>>("n_elems");

      params.set<MooseEnum>("direction") =
          DiscreteLineSegmentInterface::getAlignmentAxis(orientation);
      params.set<std::vector<Real>>("bounds") =
          DiscreteLineSegmentInterface::getElementBoundaryCoordinates(
              position, orientation, rotation, lengths, n_elems);
    }
    else
      mooseError(
          "The parameters 'position', 'orientation', 'length', and 'n_elems' must be provided.");

    _problem->addUserObject(class_name, _T_wall_user_object_name, params);
  }
}

void
CoupledHeatTransferAction::addTransfers()
{
  // Transfers to the flow channel application

  const bool skip_coordinate_collapsing = getParam<bool>("skip_coordinate_collapsing");

  {
    const std::string class_name = "MultiAppUserObjectTransfer";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<MultiAppName>("to_multi_app") = _multi_app_name;
    params.set<UserObjectName>("user_object") = {_T_wall_user_object_name};
    params.set<std::vector<AuxVariableName>>("variable") = {_T_wall_var_name};
    params.set<bool>("skip_coordinate_collapsing") = skip_coordinate_collapsing;
    _problem->addTransfer(class_name, name() + "_T_solid_transfer", params);
  }

  // Transfers from the flow channel application. Note that
  // MultiAppNearestNodeTransfer seems like it should be a more appropriate
  // choice than MultiAppUserObjectTransfer, but it has been noted that for
  // large meshes, MultiAppNearestNodeTransfer is slower than
  // MultiAppUserObjectTransfer.
  for (unsigned int k = 0; k < _n_phases; k++)
  {
    {
      const std::string class_name = "MultiAppUserObjectTransfer";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MultiAppName>("from_multi_app") = _multi_app_name;
      params.set<UserObjectName>("user_object") = _T_fluid_user_object_names[k];
      params.set<std::vector<AuxVariableName>>("variable") = {_T_fluid_var_names[k]};
      params.set<bool>("skip_coordinate_collapsing") = skip_coordinate_collapsing;
      _problem->addTransfer(class_name, name() + "_T_fluid_transfer" + std::to_string(k), params);
    }
    {
      const std::string class_name = "MultiAppUserObjectTransfer";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MultiAppName>("from_multi_app") = _multi_app_name;
      params.set<UserObjectName>("user_object") = _htc_user_object_names[k];
      params.set<std::vector<AuxVariableName>>("variable") = {_htc_var_names[k]};
      params.set<bool>("skip_coordinate_collapsing") = skip_coordinate_collapsing;
      _problem->addTransfer(class_name, name() + "_htc_transfer" + std::to_string(k), params);
    }
    if (_n_phases > 1)
    {
      const std::string class_name = "MultiAppUserObjectTransfer";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MultiAppName>("from_multi_app") = _multi_app_name;
      params.set<UserObjectName>("user_object") = _kappa_user_object_names[k];
      params.set<std::vector<AuxVariableName>>("variable") = {_kappa_var_names[k]};
      params.set<bool>("skip_coordinate_collapsing") = skip_coordinate_collapsing;
      _problem->addTransfer(class_name, name() + "_kappa_transfer" + std::to_string(k), params);
    }
  }
}
