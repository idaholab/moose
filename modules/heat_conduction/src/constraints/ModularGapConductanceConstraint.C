//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ModularGapConductanceConstraint.h"
#include "GapFluxModelBase.h"
#include "GapFluxModelPressureDependentConduction.h"

#include "MooseError.h"

#include "libmesh/parallel_algebra.h"

registerMooseObject("HeatConductionApp", ModularGapConductanceConstraint);

InputParameters
ModularGapConductanceConstraint::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addClassDescription(
      "Computes the residual and Jacobian contributions for the 'Lagrange Multiplier' "
      "implementation of the thermal contact problem. For more information, see the "
      "detailed description here: http://tinyurl.com/gmmhbe9");
  params.addParam<std::vector<UserObjectName>>("gap_flux_models",
                                               "List of GapFluxModel user objects");
  params.addCoupledVar("displacements", "Displacement variables");

  MooseEnum gap_geometry_type("AUTO PLATE CYLINDER SPHERE", "AUTO");
  params.addParam<MooseEnum>(
      "gap_geometry_type",
      gap_geometry_type,
      "Gap calculation type. The geometry type is used to compute "
      "gap distances and scale fluxes to ensure energy balance. If AUTO is selected, the gap "
      "geometry is automatically set via the mesh coordinate system.");
  params.addRangeCheckedParam<Real>("max_gap", 1.0e6, "max_gap>=0", "A maximum gap size");
  params.addParam<RealVectorValue>("cylinder_axis_point_1",
                                   "Start point for line defining cylindrical axis");
  params.addParam<RealVectorValue>("cylinder_axis_point_2",
                                   "End point for line defining cylindrical axis");
  params.addParam<RealVectorValue>("sphere_origin", "Origin for sphere geometry");

  // We should default use_displaced_mesh to true. If no displaced mesh exists
  // FEProblemBase::addConstraint will automatically correct it to false. However,
  // this will still prompt a call from AugmentSparsityOnInterface to get a displaced
  // mortar interface since object._use_displaced_mesh = true.

  return params;
}

ModularGapConductanceConstraint::ModularGapConductanceConstraint(const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _gap_flux_model_names(getParam<std::vector<UserObjectName>>("gap_flux_models")),
    _disp_name(parameters.getVecMooseType("displacements")),
    _n_disp(_disp_name.size()),
    _disp_secondary(_n_disp),
    _disp_primary(_n_disp),
    _gap_geometry_type(getParam<MooseEnum>("gap_geometry_type").getEnum<GapGeometry>()),
    _gap_width(0.0),
    _surface_integration_factor(1.0),
    _p1(declareRestartableData<Point>("cylinder_axis_point_1", Point(0, 1, 0))),
    _p2(declareRestartableData<Point>("cylinder_axis_point_2", Point(0, 0, 0))),
    _r1(0),
    _r2(0),
    _max_gap(getParam<Real>("max_gap")),
    _adjusted_length(0.0),
    _disp_x_var(getVar("displacements", 0)),
    _disp_y_var(getVar("displacements", 1)),
    _disp_z_var(_n_disp == 3 ? getVar("displacements", 2) : nullptr)
{
  if (_n_disp && !getParam<bool>("use_displaced_mesh"))
    paramWarning("displacements",
                 "You are coupling displacement variables but are evaluating the gap width on the "
                 "undisplaced mesh. This is probably a mistake.");

  for (unsigned int i = 0; i < _n_disp; ++i)
  {
    auto & disp_var = _subproblem.getStandardVariable(_tid, _disp_name[i]);
    _disp_secondary[i] = &disp_var.adSln();
    _disp_primary[i] = &disp_var.adSlnNeighbor();
  }

  const auto use_displaced_mesh = getParam<bool>("use_displaced_mesh");

  for (const auto & name : _gap_flux_model_names)
  {
    const auto & gap_model = getUserObjectByName<GapFluxModelBase>(name);

    // This constraint explicitly calls the gap flux model user objects to
    // obtain contributions to its residuals. It therefore depends on all
    // variables and material properties, that these gap flux models use, to be
    // computed and up to date. To ensure that we collect all variable and
    // material property dependencies of these models and declare them as
    // dependencies of this constraint object. This turns an implicit, hidden
    // dependency into an explicit dependency that MOOSE will automatically fulfill.

    // pass variable dependencies through
    const auto & var_dependencies = gap_model.getMooseVariableDependencies();
    for (const auto & var : var_dependencies)
      addMooseVariableDependency(var);

    // pass material property dependencies through
    const auto & mat_dependencies = gap_model.getMatPropDependencies();
    _material_property_dependencies.insert(mat_dependencies.begin(), mat_dependencies.end());

    // ensure that the constraint and the flux models operate on the same mesh
    if (gap_model.parameters().get<bool>("use_displaced_mesh") != use_displaced_mesh)
      paramError(
          "use_displaced_mesh",
          "The gap flux model '",
          name,
          "' should operate on the same mesh (displaced/undisplaced) as the constraint object");

    // add gap model to list
    _gap_flux_models.push_back(&gap_model);
  }
}

void
ModularGapConductanceConstraint::initialSetup()
{
  ///set generated from the passed in vector of subdomain names
  const auto & subdomainIDs = _mesh.meshSubdomains();

  // make sure all subdomains are using the same coordinate system
  Moose::CoordinateSystemType coord_system = feProblem().getCoordSystem(*subdomainIDs.begin());
  for (auto subdomain : subdomainIDs)
    if (feProblem().getCoordSystem(subdomain) != coord_system)
      mooseError("ModularGapConductanceConstraint requires all subdomains to have the same "
                 "coordinate system.");

  // Select proper coordinate system and geometry (plate, cylinder, sphere)
  setGapGeometryParameters(
      _pars, coord_system, feProblem().getAxisymmetricRadialCoord(), _gap_geometry_type);
}

void
ModularGapConductanceConstraint::setGapGeometryParameters(
    const InputParameters & params,
    const Moose::CoordinateSystemType coord_sys,
    unsigned int axisymmetric_radial_coord,
    GapGeometry & gap_geometry_type)
{

  // Determine what type of gap geometry we are dealing with
  // Either user input or from system's coordinate systems
  if (gap_geometry_type == GapGeometry::AUTO)
  {
    if (coord_sys == Moose::COORD_XYZ)
      gap_geometry_type = GapGeometry::PLATE;
    else if (coord_sys == Moose::COORD_RZ)
      gap_geometry_type = GapGeometry::CYLINDER;
    else if (coord_sys == Moose::COORD_RSPHERICAL)
      gap_geometry_type = GapGeometry::SPHERE;
    else
      mooseError("Internal Error");
  }

  if (params.isParamValid("cylinder_axis_point_1") != params.isParamValid("cylinder_axis_point_2"))
    paramError(
        "cylinder_axis_point_1",
        "Either specify both `cylinder_axis_point_1` and `cylinder_axis_point_2` or neither.");

  // Check consistency of geometry information
  // Inform the user of needed input according to gap geometry (if not PLATE)
  if (gap_geometry_type == GapGeometry::PLATE)
  {
    if (coord_sys == Moose::COORD_RSPHERICAL)
      paramError("gap_geometry_type",
                 "'gap_geometry_type = PLATE' cannot be used with models having a spherical "
                 "coordinate system.");
  }
  else if (gap_geometry_type == GapGeometry::CYLINDER)
  {
    if (coord_sys == Moose::COORD_XYZ)
    {
      if (params.isParamValid("cylinder_axis_point_1") &&
          params.isParamValid("cylinder_axis_point_2"))
      {
        _p1 = params.get<RealVectorValue>("cylinder_axis_point_1");
        _p2 = params.get<RealVectorValue>("cylinder_axis_point_2");
      }
      else if (_mesh.dimension() == 3)
        paramError("gap_geometry_type",
                   "For 'gap_geometry_type = CYLINDER' to be used with a Cartesian model in 3D, "
                   "'cylinder_axis_point_1' and 'cylinder_axis_point_2' must be specified.");
      else
      {
        deduceGeometryParameters();
        mooseInfoRepeated(
            "ModularGapConductanceConstraint '", name(), "' deduced cylinder axis as ", _p1, _p2);
      }
    }
    else if (coord_sys == Moose::COORD_RZ)
    {
      if (params.isParamValid("cylinder_axis_point_1") ||
          params.isParamValid("cylinder_axis_point_2"))
        paramError("cylinder_axis_point_1",
                   "The 'cylinder_axis_point_1' and 'cylinder_axis_point_2' cannot be specified "
                   "with axisymmetric models.  The y-axis is used as the cylindrical axis of "
                   "symmetry.");

      if (axisymmetric_radial_coord == 0) // R-Z problem
      {
        _p1 = Point(0, 0, 0);
        _p2 = Point(0, 1, 0);
      }
      else // Z-R problem
      {
        _p1 = Point(0, 0, 0);
        _p2 = Point(1, 0, 0);
      }
    }
    else if (coord_sys == Moose::COORD_RSPHERICAL)
      paramError("gap_geometry_type",
                 "'gap_geometry_type = CYLINDER' cannot be used with models having a spherical "
                 "coordinate system.");
  }
  else if (gap_geometry_type == GapGeometry::SPHERE)
  {
    if (coord_sys == Moose::COORD_XYZ || coord_sys == Moose::COORD_RZ)
    {
      if (params.isParamValid("sphere_origin"))
        _p1 = params.get<RealVectorValue>("sphere_origin");
      else if (coord_sys == Moose::COORD_XYZ)
      {
        deduceGeometryParameters();
        mooseInfoRepeated(
            "ModularGapConductanceConstraint '", name(), "' deduced sphere origin as ", _p1);
      }
      else
        paramError("gap_geometry_type",
                   "For 'gap_geometry_type = SPHERE' to be used with an axisymmetric "
                   "model, 'sphere_origin' must be specified.");
    }
    else if (coord_sys == Moose::COORD_RSPHERICAL)
    {
      if (params.isParamValid("sphere_origin"))
        paramError("sphere_origin",
                   "The 'sphere_origin' cannot be specified with spherical models.  x=0 is used "
                   "as the spherical origin.");
      _p1 = Point(0, 0, 0);
    }
  }
}

ADReal
ModularGapConductanceConstraint::computeSurfaceIntegrationFactor() const
{

  ADReal surface_integration_factor = 1.0;

  if (_gap_geometry_type == GapGeometry::CYLINDER)
    surface_integration_factor = 0.5 * (_r1 + _r2) / _radius;
  else if (_gap_geometry_type == GapGeometry::SPHERE)
    surface_integration_factor = 0.25 * (_r1 + _r2) * (_r1 + _r2) / (_radius * _radius);

  return surface_integration_factor;
}

ADReal
ModularGapConductanceConstraint::computeGapLength() const
{

  if (_gap_geometry_type == GapGeometry::CYLINDER)
  {
    const auto denominator = _radius * std::log(_r2 / _r1);
    return std::min(denominator, _max_gap);
  }
  else if (_gap_geometry_type == GapGeometry::SPHERE)
  {
    const auto denominator = _radius * _radius * ((1.0 / _r1) - (1.0 / _r2));
    return std::min(denominator, _max_gap);
  }
  else
    return std::min(_r2 - _r1, _max_gap);
}

void
ModularGapConductanceConstraint::computeGapRadii(const ADReal & gap_length)
{
  const Point & current_point = _q_point[_qp];

  if (_gap_geometry_type == GapGeometry::CYLINDER)
  {
    // The vector _p1 + t*(_p2-_p1) defines the cylindrical axis.  The point along this
    // axis closest to current_point is found by the following for t:
    const Point p2p1(_p2 - _p1);
    const Point p1pc(_p1 - current_point);
    const Real t = -(p1pc * p2p1) / p2p1.norm_sq();

    // The nearest point on the cylindrical axis to current_point is p.
    const Point p(_p1 + t * p2p1);
    Point rad_vec(current_point - p);
    Real rad = rad_vec.norm();
    rad_vec /= rad;
    Real rad_dot_norm = rad_vec * _normals[_qp];

    if (rad_dot_norm > 0)
    {
      _r1 = rad;
      _r2 = rad + gap_length;
      _radius = _r1;
    }
    else if (rad_dot_norm < 0)
    {
      _r1 = rad - gap_length;
      _r2 = rad;
      _radius = _r2;
    }
    else
      mooseError("Issue with cylindrical flux calculation normals.\n");
  }
  else if (_gap_geometry_type == GapGeometry::SPHERE)
  {
    const Point origin_to_curr_point(current_point - _p1);
    const Real normal_dot = origin_to_curr_point * _normals[_qp];
    const Real curr_point_radius = origin_to_curr_point.norm();
    if (normal_dot > 0) // on inside surface
    {
      _r1 = curr_point_radius;
      _r2 = curr_point_radius + gap_length;
      _radius = _r1;
    }
    else if (normal_dot < 0) // on outside surface
    {
      _r1 = curr_point_radius - gap_length;
      _r2 = curr_point_radius;
      _radius = _r2;
    }
    else
      mooseError("Issue with spherical flux calculation normals. \n");
  }
  else
  {
    _r2 = gap_length;
    _r1 = 0;
    _radius = 0;
  }
}

ADReal
ModularGapConductanceConstraint::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Primary:
      return _lambda[_qp] * _test_primary[_i][_qp];

    case Moose::MortarType::Secondary:
      return -_lambda[_qp] * _test_secondary[_i][_qp];

    case Moose::MortarType::Lower:
    {
      // we are creating an AD version of phys points primary and secondary here...
      ADRealVectorValue ad_phys_points_primary = _phys_points_primary[_qp];
      ADRealVectorValue ad_phys_points_secondary = _phys_points_secondary[_qp];

      // ...which uses the derivative vector of the primary and secondary displacements as
      // an approximation of the true phys points derivatives when the mesh is displacing
      if (_displaced)
      {
        // Trim interior node variable derivatives
        const auto & primary_ip_lowerd_map = amg().getPrimaryIpToLowerElementMap(
            *_lower_primary_elem, *_lower_primary_elem->interior_parent(), *_lower_secondary_elem);
        const auto & secondary_ip_lowerd_map =
            amg().getSecondaryIpToLowerElementMap(*_lower_secondary_elem);

        std::array<const MooseVariable *, 3> var_array{{_disp_x_var, _disp_y_var, _disp_z_var}};
        std::array<ADReal, 3> primary_disp;
        std::array<ADReal, 3> secondary_disp;

        for (unsigned int i = 0; i < _n_disp; ++i)
        {
          primary_disp[i] = (*_disp_primary[i])[_qp];
          secondary_disp[i] = (*_disp_secondary[i])[_qp];
        }

        trimInteriorNodeDerivatives(primary_ip_lowerd_map, var_array, primary_disp, false);
        trimInteriorNodeDerivatives(secondary_ip_lowerd_map, var_array, secondary_disp, true);

        // Populate quantities with trimmed derivatives
        for (unsigned int i = 0; i < _n_disp; ++i)
        {
          ad_phys_points_primary(i).derivatives() = primary_disp[i].derivatives();
          ad_phys_points_secondary(i).derivatives() = secondary_disp[i].derivatives();
        }
      }

      // compute an ADReal gap width to pass to each gap flux model
      _gap_width = (ad_phys_points_primary - ad_phys_points_secondary) * _normals[_qp];

      // First, compute radii with _gap_width
      computeGapRadii(_gap_width);

      // With radii, compute adjusted gap length for conduction
      _adjusted_length = computeGapLength();

      // Ensure energy balance for non-flat (non-PLATE) general geometries when using radiation
      _surface_integration_factor = computeSurfaceIntegrationFactor();

      // Sum up all flux contributions from all supplied gap flux models
      ADReal flux = 0.0;
      for (const auto & model : _gap_flux_models)
        flux += model->computeFluxInternal(*this);

      // The Lagrange multiplier _is_ the gap flux
      return (_lambda[_qp] - flux) * _test[_i][_qp];
    }

    default:
      return 0;
  }
}

void
ModularGapConductanceConstraint::deduceGeometryParameters()
{
  Point position;
  Real area = 0.0;
  const auto my_pid = processor_id();

  // build side element list as (element, side, id) tuples
  const auto bnd = _mesh.buildActiveSideList();

  std::unique_ptr<const Elem> side_ptr;
  for (auto [elem_id, side, id] : bnd)
    if (id == _primary_id)
    {
      const auto * elem = _mesh.elemPtr(elem_id);
      if (elem->processor_id() != my_pid)
        continue;

      // update side_ptr
      elem->side_ptr(side_ptr, side);

      // area of the (linearized) side
      const auto side_area = side_ptr->volume();

      // position of the side
      const auto side_position = side_ptr->true_centroid();

      // sum up
      position += side_position * side_area;
      area += side_area;
    }

  // parallel communication
  _communicator.sum(position);
  _communicator.sum(area);

  // set axis
  if (area == 0.0)
  {
    if (_gap_geometry_type == GapGeometry::CYLINDER)
      paramError("gap_geometry_type",
                 "Unable to decuce cylinder axis automatically, please specify "
                 "'cylinder_axis_point_1' and 'cylinder_axis_point_2'.");
    else if (_gap_geometry_type == GapGeometry::SPHERE)
      paramError("gap_geometry_type",
                 "Unable to decuce sphere origin automatically, please specify "
                 "'sphere_origin'.");
    else
      mooseError("Internal error.");
  }

  _p1 = position / area;
  _p2 = _p1 + Point(0, 0, 1);
}
