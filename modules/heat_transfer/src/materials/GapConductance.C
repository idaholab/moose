//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapConductance.h"

// MOOSE includes
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "PenetrationLocator.h"
#include "SystemBase.h"
#include "AddVariableAction.h"

#include "libmesh/string_to_enum.h"

registerMooseObject("HeatConductionApp", GapConductance);

InputParameters
GapConductance::validParams()
{
  InputParameters params = Material::validParams();
  params += GapConductance::actionParameters();

  params.addRequiredCoupledVar("variable", "Temperature variable");

  // Node based
  params.addCoupledVar("gap_distance", "Distance across the gap");
  params.addCoupledVar("gap_temp", "Temperature on the other side of the gap");
  params.addParam<Real>("gap_conductivity", 1.0, "The thermal conductivity of the gap material");
  params.addParam<FunctionName>(
      "gap_conductivity_function",
      "Thermal conductivity of the gap material as a function.  Multiplied by gap_conductivity.");
  params.addCoupledVar("gap_conductivity_function_variable",
                       "Variable to be used in the gap_conductivity_function in place of time");

  // Quadrature based
  params.addParam<bool>("quadrature",
                        false,
                        "Whether or not to do quadrature point based gap heat "
                        "transfer.  If this is true then gap_distance and "
                        "gap_temp should NOT be provided (and will be "
                        "ignored); however, paired_boundary and variable are "
                        "then required.");
  params.addParam<BoundaryName>("paired_boundary", "The boundary to be penetrated");

  params.addParam<Real>("stefan_boltzmann", 5.669e-8, "The Stefan-Boltzmann constant");

  params.addParam<bool>("use_displaced_mesh",
                        true,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that in "
                        "the case this is true but no displacements "
                        "are provided in the Mesh block the "
                        "undisplaced mesh will still be used.");

  params.addParam<bool>(
      "warnings", false, "Whether to output warning messages concerning nodes not being found");

  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<MooseEnum>("order", orders, "The finite element order");

  return params;
}

InputParameters
GapConductance::actionParameters()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::string>(
      "appended_property_name", "", "Name appended to material properties to make them unique");
  MooseEnum gap_geom_types("PLATE CYLINDER SPHERE");
  params.addParam<MooseEnum>("gap_geometry_type", gap_geom_types, "Gap calculation type.");

  params.addParam<RealVectorValue>("cylinder_axis_point_1",
                                   "Start point for line defining cylindrical axis");
  params.addParam<RealVectorValue>("cylinder_axis_point_2",
                                   "End point for line defining cylindrical axis");
  params.addParam<RealVectorValue>("sphere_origin", "Origin for sphere geometry");

  params.addRangeCheckedParam<Real>("emissivity_primary",
                                    1,
                                    "emissivity_primary>=0 & emissivity_primary<=1",
                                    "The emissivity of the primary surface");
  params.addRangeCheckedParam<Real>("emissivity_secondary",
                                    1,
                                    "emissivity_secondary>=0 & emissivity_secondary<=1",
                                    "The emissivity of the secondary surface");
  // Common
  params.addRangeCheckedParam<Real>(
      "min_gap", 1e-6, "min_gap>0", "A minimum gap (denominator) size");
  params.addRangeCheckedParam<Real>(
      "max_gap", 1e6, "max_gap>=0", "A maximum gap (denominator) size");
  params.addRangeCheckedParam<unsigned int>(
      "min_gap_order", 0, "min_gap_order<=1", "Order of the Taylor expansion below min_gap");

  return params;
}

GapConductance::GapConductance(const InputParameters & parameters)
  : Material(parameters),
    _appended_property_name(getParam<std::string>("appended_property_name")),
    _temp(coupledValue("variable")),
    _gap_geometry_type(declareRestartableData<GapConductance::GAP_GEOMETRY>("gap_geometry_type",
                                                                            GapConductance::PLATE)),
    _quadrature(getParam<bool>("quadrature")),
    _gap_temp(0),
    _gap_distance(88888),
    _radius(0),
    _r1(0),
    _r2(0),
    _has_info(false),
    _gap_distance_value(_quadrature ? _zero : coupledValue("gap_distance")),
    _gap_temp_value(_quadrature ? _zero : coupledValue("gap_temp")),
    _gap_conductance(declareProperty<Real>("gap_conductance" + _appended_property_name)),
    _gap_conductance_dT(declareProperty<Real>("gap_conductance" + _appended_property_name + "_dT")),
    _gap_thermal_conductivity(declareProperty<Real>("gap_conductivity")),
    _gap_conductivity(getParam<Real>("gap_conductivity")),
    _gap_conductivity_function(isParamValid("gap_conductivity_function")
                                   ? &getFunction("gap_conductivity_function")
                                   : nullptr),
    _gap_conductivity_function_variable(isCoupled("gap_conductivity_function_variable")
                                            ? &coupledValue("gap_conductivity_function_variable")
                                            : nullptr),
    _stefan_boltzmann(getParam<Real>("stefan_boltzmann")),
    _min_gap(getParam<Real>("min_gap")),
    _min_gap_order(getParam<unsigned int>("min_gap_order")),
    _max_gap(getParam<Real>("max_gap")),
    _temp_var(_quadrature ? getVar("variable", 0) : nullptr),
    _penetration_locator(nullptr),
    _serialized_solution(_quadrature ? &_temp_var->sys().currentSolution() : nullptr),
    _dof_map(_quadrature ? &_temp_var->sys().dofMap() : nullptr),
    _warnings(getParam<bool>("warnings")),
    _p1(declareRestartableData<Point>("cylinder_axis_point_1", Point(0, 1, 0))),
    _p2(declareRestartableData<Point>("cylinder_axis_point_2", Point(0, 0, 0)))
{
  // Set emissivity but allow legacy naming; legacy names are used if they
  // are present
  const auto emissivity_primary = getParam<Real>("emissivity_primary");
  const auto emissivity_secondary = getParam<Real>("emissivity_secondary");

  _emissivity = emissivity_primary != 0.0 && emissivity_secondary != 0.0
                    ? 1.0 / emissivity_primary + 1.0 / emissivity_secondary - 1
                    : 0.0;

  if (_quadrature)
  {
    if (!parameters.isParamValid("paired_boundary"))
      mooseError("No 'paired_boundary' provided for ", _name);

    _penetration_locator = &_subproblem.geomSearchData().getQuadraturePenetrationLocator(
        parameters.get<BoundaryName>("paired_boundary"),
        getParam<std::vector<BoundaryName>>("boundary")[0],
        Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")));
  }
  else
  {
    if (!isCoupled("gap_distance"))
      paramError("gap_distance", "needed if not using quadrature point based gap heat");

    if (!isCoupled("gap_temp"))
      paramError("gap_temp", "needed if not using quadrature point based gap heat");
  }

  if (_mesh.uniformRefineLevel() != 0)
    mooseError("GapConductance does not work with uniform mesh refinement.");
}

void
GapConductance::initialSetup()
{
  ///set generated from the passed in vector of subdomain names
  const auto & check_subdomains =
      blockRestricted() && !blockIDs().empty() ? blockIDs() : meshBlockIDs();
  if (check_subdomains.empty())
    mooseError("No subdomains found");

  // make sure all subdomains are using the same coordinate system
  Moose::CoordinateSystemType coord_system = _fe_problem.getCoordSystem(*check_subdomains.begin());
  for (auto subdomain : check_subdomains)
    if (_fe_problem.getCoordSystem(subdomain) != coord_system)
      mooseError(
          "The GapConductance model requires all subdomains to have the same coordinate system.");

  // Select proper coordinate system and geometry (plate, cylinder, spheres)
  setGapGeometryParameters(
      _pars, coord_system, _fe_problem.getAxisymmetricRadialCoord(), _gap_geometry_type, _p1, _p2);
}

void
GapConductance::setGapGeometryParameters(const InputParameters & params,
                                         const Moose::CoordinateSystemType coord_sys,
                                         unsigned int axisymmetric_radial_coord,
                                         GAP_GEOMETRY & gap_geometry_type,
                                         Point & p1,
                                         Point & p2)
{
  if (params.isParamSetByUser("gap_geometry_type"))
  {
    gap_geometry_type =
        GapConductance::GAP_GEOMETRY(int(params.get<MooseEnum>("gap_geometry_type")));
  }
  else
  {
    if (coord_sys == Moose::COORD_XYZ)
      gap_geometry_type = GapConductance::PLATE;
    else if (coord_sys == Moose::COORD_RZ)
      gap_geometry_type = GapConductance::CYLINDER;
    else if (coord_sys == Moose::COORD_RSPHERICAL)
      gap_geometry_type = GapConductance::SPHERE;
  }

  if (gap_geometry_type == GapConductance::PLATE)
  {
    if (coord_sys == Moose::COORD_RSPHERICAL)
      ::mooseError("'gap_geometry_type = PLATE' cannot be used with models having a spherical "
                   "coordinate system.");
  }
  else if (gap_geometry_type == GapConductance::CYLINDER)
  {
    if (coord_sys == Moose::COORD_XYZ)
    {
      if (!params.isParamValid("cylinder_axis_point_1") ||
          !params.isParamValid("cylinder_axis_point_2"))
        ::mooseError("For 'gap_geometry_type = CYLINDER' to be used with a Cartesian model, "
                     "'cylinder_axis_point_1' and 'cylinder_axis_point_2' must be specified.");
      p1 = params.get<RealVectorValue>("cylinder_axis_point_1");
      p2 = params.get<RealVectorValue>("cylinder_axis_point_2");
    }
    else if (coord_sys == Moose::COORD_RZ)
    {
      if (params.isParamValid("cylinder_axis_point_1") ||
          params.isParamValid("cylinder_axis_point_2"))
        ::mooseError("The 'cylinder_axis_point_1' and 'cylinder_axis_point_2' cannot be specified "
                     "with axisymmetric models.  The y-axis is used as the cylindrical axis of "
                     "symmetry.");

      if (axisymmetric_radial_coord == 0) // R-Z problem
      {
        p1 = Point(0, 0, 0);
        p2 = Point(0, 1, 0);
      }
      else // Z-R problem
      {
        p1 = Point(0, 0, 0);
        p2 = Point(1, 0, 0);
      }
    }
    else if (coord_sys == Moose::COORD_RSPHERICAL)
      ::mooseError("'gap_geometry_type = CYLINDER' cannot be used with models having a spherical "
                   "coordinate system.");
  }
  else if (gap_geometry_type == GapConductance::SPHERE)
  {
    if (coord_sys == Moose::COORD_XYZ || coord_sys == Moose::COORD_RZ)
    {
      if (!params.isParamValid("sphere_origin"))
        ::mooseError("For 'gap_geometry_type = SPHERE' to be used with a Cartesian or axisymmetric "
                     "model, 'sphere_origin' must be specified.");
      p1 = params.get<RealVectorValue>("sphere_origin");
    }
    else if (coord_sys == Moose::COORD_RSPHERICAL)
    {
      if (params.isParamValid("sphere_origin"))
        ::mooseError("The 'sphere_origin' cannot be specified with spherical models.  x=0 is used "
                     "as the spherical origin.");
      p1 = Point(0, 0, 0);
    }
  }
}

void
GapConductance::computeQpProperties()
{
  computeGapValues();
  computeQpConductance();
}

void
GapConductance::computeQpConductance()
{
  if (_has_info)
  {
    _gap_conductance[_qp] = h_conduction() + h_radiation();
    _gap_conductance_dT[_qp] = dh_conduction() + dh_radiation();
  }
  else
  {
    _gap_conductance[_qp] = 0;
    _gap_conductance_dT[_qp] = 0;
  }
}

Real
GapConductance::gapAttenuation(Real adjusted_length, Real min_gap, unsigned int min_gap_order)
{
  mooseAssert(min_gap > 0, "min_gap must be larger than zero.");

  if (adjusted_length > min_gap)
    return 1.0 / adjusted_length;
  else
    switch (min_gap_order)
    {
      case 0:
        return 1.0 / min_gap;

      case 1:
        return 1.0 / min_gap - (adjusted_length - min_gap) / (min_gap * min_gap);

      default:
        ::mooseError("Invalid Taylor expansion order");
    }
}

Real
GapConductance::h_conduction()
{
  _gap_thermal_conductivity[_qp] = gapK();
  const Real adjusted_length = gapLength(_gap_geometry_type, _radius, _r1, _r2, _max_gap);
  return _gap_thermal_conductivity[_qp] * gapAttenuation(adjusted_length, _min_gap, _min_gap_order);
}

Real
GapConductance::dh_conduction()
{
  return 0.0;
}

Real
GapConductance::h_radiation()
{
  /*
   Gap conductance due to radiation is based on the diffusion approximation:

      qr = sigma*Fe*(Tf^4 - Tc^4) ~ hr(Tf - Tc)
         where sigma is the Stefan-Boltzmann constant, Fe is an emissivity function, Tf and Tc
         are the fuel and clad absolute temperatures, respectively, and hr is the radiant gap
         conductance. Solving for hr,

      hr = sigma*Fe*(Tf^4 - Tc^4) / (Tf - Tc)
         which can be factored to give:

      hr = sigma*Fe*(Tf^2 + Tc^2) * (Tf + Tc)

   Approximating the fuel-clad gap as infinite parallel planes, the emissivity function is given by:

      Fe = 1 / (1/ef + 1/ec - 1)
  */

  if (_emissivity == 0.0)
    return 0.0;

  // We add 'surface_integration_factor' to account for the surface integration of the conductance
  // due to radiation.
  Real surface_integration_factor = 1.0;

  if (_gap_geometry_type == GapConductance::CYLINDER)
    surface_integration_factor = 0.5 * (_r1 + _r2) / _radius;
  else if (_gap_geometry_type == GapConductance::SPHERE)
    surface_integration_factor = 0.25 * (_r1 + _r2) * (_r1 + _r2) / (_radius * _radius);

  const Real temp_func =
      (_temp[_qp] * _temp[_qp] + _gap_temp * _gap_temp) * (_temp[_qp] + _gap_temp);

  return _stefan_boltzmann * temp_func / _emissivity * surface_integration_factor;
}

Real
GapConductance::dh_radiation()
{
  if (_emissivity == 0.0)
    return 0.0;

  Real surface_integration_factor = 1.0;

  if (_gap_geometry_type == GapConductance::CYLINDER)
    surface_integration_factor = 0.5 * (_r1 + _r2) / _radius;
  else if (_gap_geometry_type == GapConductance::SPHERE)
    surface_integration_factor = 0.25 * (_r1 + _r2) * (_r1 + _r2) / (_radius * _radius);

  const Real temp_func = 3 * _temp[_qp] * _temp[_qp] + _gap_temp * (2 * _temp[_qp] + _gap_temp);

  return _stefan_boltzmann * temp_func / _emissivity * surface_integration_factor;
}

Real
GapConductance::gapLength(const GapConductance::GAP_GEOMETRY & gap_geom,
                          const Real radius,
                          const Real r1,
                          const Real r2,
                          const Real max_gap)
{
  if (gap_geom == GapConductance::CYLINDER)
    return gapCyl(radius, r1, r2, max_gap);
  else if (gap_geom == GapConductance::SPHERE)
    return gapSphere(radius, r1, r2, max_gap);
  else
    return gapRect(r2 - r1, max_gap);
}

Real
GapConductance::gapRect(const Real distance, const Real max_gap)
{
  return std::min(distance, max_gap);
}

Real
GapConductance::gapCyl(const Real radius, const Real r1, const Real r2, const Real max_denom)
{
  const Real denominator = radius * std::log(r2 / r1);
  return std::min(denominator, max_denom);
}

Real
GapConductance::gapSphere(const Real radius, const Real r1, const Real r2, const Real max_denom)
{
  const Real denominator = radius * radius * ((1.0 / r1) - (1.0 / r2));
  return std::min(denominator, max_denom);
}

Real
GapConductance::gapK()
{
  Real gap_conductivity = _gap_conductivity;

  if (_gap_conductivity_function)
  {
    if (_gap_conductivity_function_variable)
      gap_conductivity *= _gap_conductivity_function->value(
          (*_gap_conductivity_function_variable)[_qp], _q_point[_qp]);
    else
      gap_conductivity *= _gap_conductivity_function->value(_t, _q_point[_qp]);
  }

  return gap_conductivity;
}

void
GapConductance::computeGapValues()
{
  if (!_quadrature)
  {
    _has_info = true;
    _gap_temp = _gap_temp_value[_qp];
    _gap_distance = _gap_distance_value[_qp];
  }
  else
  {
    Node * qnode = _mesh.getQuadratureNode(_current_elem, _current_side, _qp);
    PenetrationInfo * pinfo = _penetration_locator->_penetration_info[qnode->id()];

    _gap_temp = 0.0;
    _gap_distance = 88888;
    _has_info = false;

    if (pinfo)
    {
      _gap_distance = pinfo->_distance;
      _has_info = true;

      const Elem * secondary_side = pinfo->_side;
      std::vector<std::vector<Real>> & secondary_side_phi = pinfo->_side_phi;
      std::vector<dof_id_type> secondary_side_dof_indices;

      _dof_map->dof_indices(secondary_side, secondary_side_dof_indices, _temp_var->number());

      for (unsigned int i = 0; i < secondary_side_dof_indices.size(); ++i)
      {
        // The zero index is because we only have one point that the phis are evaluated at
        _gap_temp +=
            secondary_side_phi[i][0] * (*(*_serialized_solution))(secondary_side_dof_indices[i]);
      }
    }
    else
    {
      if (_warnings)
        mooseWarning("No gap value information found for node ",
                     qnode->id(),
                     " on processor ",
                     processor_id(),
                     " at coordinate ",
                     Point(*qnode));
    }
  }

  Point current_point(_q_point[_qp]);
  computeGapRadii(
      _gap_geometry_type, current_point, _p1, _p2, _gap_distance, _normals[_qp], _r1, _r2, _radius);
}

void
GapConductance::computeGapRadii(const GapConductance::GAP_GEOMETRY gap_geometry_type,
                                const Point & current_point,
                                const Point & p1,
                                const Point & p2,
                                const Real & gap_distance,
                                const Point & current_normal,
                                Real & r1,
                                Real & r2,
                                Real & radius)
{
  if (gap_geometry_type == GapConductance::CYLINDER)
  {
    // The vector _p1 + t*(_p2-_p1) defines the cylindrical axis.  The point along this
    // axis closest to current_point is found by the following for t:
    const Point p2p1(p2 - p1);
    const Point p1pc(p1 - current_point);
    const Real t = -(p1pc * p2p1) / p2p1.norm_sq();

    // The nearest point on the cylindrical axis to current_point is p.
    const Point p(p1 + t * p2p1);
    Point rad_vec(current_point - p);
    Real rad = rad_vec.norm();
    rad_vec /= rad;
    Real rad_dot_norm = rad_vec * current_normal;

    if (rad_dot_norm > 0)
    {
      r1 = rad;
      r2 = rad - gap_distance; // note, gap_distance is negative
      radius = r1;
    }
    else if (rad_dot_norm < 0)
    {
      r1 = rad + gap_distance;
      r2 = rad;
      radius = r2;
    }
    else
      ::mooseError("Issue with cylindrical flux calc. normals.\n");
  }
  else if (gap_geometry_type == GapConductance::SPHERE)
  {
    const Point origin_to_curr_point(current_point - p1);
    const Real normal_dot = origin_to_curr_point * current_normal;
    const Real curr_point_radius = origin_to_curr_point.norm();
    if (normal_dot > 0) // on inside surface
    {
      r1 = curr_point_radius;
      r2 = curr_point_radius - gap_distance; // gap_distance is negative
      radius = r1;
    }
    else if (normal_dot < 0) // on outside surface
    {
      r1 = curr_point_radius + gap_distance; // gap_distance is negative
      r2 = curr_point_radius;
      radius = r2;
    }
    else
      ::mooseError("Issue with spherical flux calc. normals. \n");
  }
  else
  {
    r2 = -gap_distance;
    r1 = 0;
    radius = 0;
  }
}
