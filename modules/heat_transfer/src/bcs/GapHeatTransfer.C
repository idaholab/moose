//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapHeatTransfer.h"

// MOOSE includes
#include "AddVariableAction.h"
#include "Assembly.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "PenetrationLocator.h"
#include "SystemBase.h"
#include "GhostBoundary.h"

#include "libmesh/string_to_enum.h"

registerMooseObject("HeatTransferApp", GapHeatTransfer);

InputParameters
GapHeatTransfer::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription("Transfers heat across a gap between two "
                             "surfaces dependent on the gap geometry specified.");
  params.addParam<std::string>(
      "appended_property_name", "", "Name appended to material properties to make them unique");

  // Common
  params.addParam<Real>("min_gap", 1.0e-6, "A minimum gap size");
  params.addParam<Real>("max_gap", 1.0e6, "A maximum gap size");
  params.addRangeCheckedParam<unsigned int>(
      "min_gap_order", 0, "min_gap_order<=1", "Order of the Taylor expansion below min_gap");

  // Deprecated parameter
  MooseEnum coord_types("default XYZ cyl", "default");
  params.addDeprecatedParam<MooseEnum>(
      "coord_type",
      coord_types,
      "Gap calculation type (default or XYZ).",
      "The functionality of this parameter is replaced by 'gap_geometry_type'.");

  MooseEnum gap_geom_types("PLATE CYLINDER SPHERE");
  params.addParam<MooseEnum>("gap_geometry_type",
                             gap_geom_types,
                             "Gap calculation type. Choices are: " + gap_geom_types.getRawNames());

  params.addParam<RealVectorValue>("cylinder_axis_point_1",
                                   "Start point for line defining cylindrical axis");
  params.addParam<RealVectorValue>("cylinder_axis_point_2",
                                   "End point for line defining cylindrical axis");
  params.addParam<RealVectorValue>("sphere_origin", "Origin for sphere geometry");

  // Quadrature based
  params.addParam<bool>("quadrature",
                        false,
                        "Whether or not to do Quadrature point based gap heat "
                        "transfer.  If this is true then gap_distance and "
                        "gap_temp should NOT be provided (and will be "
                        "ignored) however paired_boundary IS then required.");
  params.addParam<BoundaryName>("paired_boundary", "The boundary to be penetrated");

  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<MooseEnum>("order", orders, "The finite element order");

  params.addParam<bool>(
      "warnings", false, "Whether to output warning messages concerning nodes not being found");

  params.addCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");

  // Node based options
  params.addCoupledVar("gap_distance", "Distance across the gap");
  params.addCoupledVar("gap_temp", "Temperature on the other side of the gap");

  params.addRelationshipManager(
      "GhostBoundary",
      Moose::RelationshipManagerType::GEOMETRIC,
      [](const InputParameters & obj_params, InputParameters & rm_params)
      {
        auto & boundary = rm_params.set<std::vector<BoundaryName>>("boundary");
        boundary = obj_params.get<std::vector<BoundaryName>>("boundary");
        boundary.push_back(obj_params.get<BoundaryName>("paired_boundary"));
      });

  return params;
}

GapHeatTransfer::GapHeatTransfer(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _gap_geometry_type(declareRestartableData<GapConductance::GAP_GEOMETRY>("gap_geometry_type",
                                                                            GapConductance::PLATE)),
    _quadrature(getParam<bool>("quadrature")),
    _secondary_flux(!_quadrature ? &_sys.getVector("secondary_flux") : NULL),
    _gap_conductance(getMaterialProperty<Real>("gap_conductance" +
                                               getParam<std::string>("appended_property_name"))),
    _gap_conductance_dT(getMaterialProperty<Real>(
        "gap_conductance" + getParam<std::string>("appended_property_name") + "_dT")),
    _min_gap(getParam<Real>("min_gap")),
    _min_gap_order(getParam<unsigned int>("min_gap_order")),
    _max_gap(getParam<Real>("max_gap")),
    _gap_temp(0),
    _gap_distance(std::numeric_limits<Real>::max()),
    _edge_multiplier(1.0),
    _has_info(false),
    _disp_vars(3, libMesh::invalid_uint),
    _gap_distance_value(_quadrature ? _zero : coupledValue("gap_distance")),
    _gap_temp_value(_quadrature ? _zero : coupledValue("gap_temp")),
    _penetration_locator(
        !_quadrature ? NULL
                     : &getQuadraturePenetrationLocator(
                           parameters.get<BoundaryName>("paired_boundary"),
                           getParam<std::vector<BoundaryName>>("boundary")[0],
                           Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")))),
    _warnings(getParam<bool>("warnings")),
    _p1(declareRestartableData<Point>("cylinder_axis_point_1", Point(0, 1, 0))),
    _p2(declareRestartableData<Point>("cylinder_axis_point_2", Point(0, 0, 0))),
    _pinfo(nullptr),
    _secondary_side_phi(nullptr),
    _secondary_side(nullptr),
    _secondary_j(0)
{
  if (isParamValid("displacements"))
  {
    // modern parameter scheme for displacements
    for (unsigned int i = 0; i < coupledComponents("displacements"); ++i)
      _disp_vars[i] = coupled("displacements", i);
  }

  if (_quadrature)
  {
    if (!parameters.isParamValid("paired_boundary"))
      mooseError(std::string("No 'paired_boundary' provided for ") + _name);
  }
  else
  {
    if (!isCoupled("gap_distance"))
      mooseError(std::string("No 'gap_distance' provided for ") + _name);

    if (!isCoupled("gap_temp"))
      mooseError(std::string("No 'gap_temp' provided for ") + _name);
  }
}

void
GapHeatTransfer::initialSetup()
{
  std::set<SubdomainID> subdomain_ids;

  for (const auto & bnd_elem : *_mesh.getBoundaryElementRange())
  {
    Elem * elem = bnd_elem->_elem;
    subdomain_ids.insert(elem->subdomain_id());
  }

  if (subdomain_ids.empty())
    mooseError("No boundary elements found");

  Moose::CoordinateSystemType coord_system = _fe_problem.getCoordSystem(*subdomain_ids.begin());

  for (auto sid : subdomain_ids)
    if (_fe_problem.getCoordSystem(sid) != coord_system)
      mooseError("The GapHeatTransfer model requires all boundary elements to have the same "
                 "coordinate system.");

  GapConductance::setGapGeometryParameters(
      _pars, coord_system, _fe_problem.getAxisymmetricRadialCoord(), _gap_geometry_type, _p1, _p2);
}

Real
GapHeatTransfer::computeQpResidual()
{
  computeGapValues();

  if (!_has_info)
    return 0.0;

  Real grad_t = (_u[_qp] - _gap_temp) * _edge_multiplier * _gap_conductance[_qp];

  // This is keeping track of this residual contribution so it can be used as the flux on the other
  // side of the gap.
  if (!_quadrature)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mutex);
    const Real secondary_flux = computeSecondaryFluxContribution(grad_t);
    _secondary_flux->add(_var.dofIndices()[_i], secondary_flux);
  }

  return _test[_i][_qp] * grad_t;
}

Real
GapHeatTransfer::computeSecondaryFluxContribution(Real grad_t)
{
  return _coord[_qp] * _JxW[_qp] * _test[_i][_qp] * grad_t;
}

void
GapHeatTransfer::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // compute this up front because it only depends on the quadrature point
    computeGapValues();

    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();

    // Ok now do the contribution from the secondary side
    if (_quadrature && _has_info)
    {
      std::vector<dof_id_type> secondary_side_dof_indices;

      _sys.dofMap().dof_indices(_secondary_side, secondary_side_dof_indices, _var.number());

      DenseMatrix<Number> K_secondary(_var.dofIndices().size(), secondary_side_dof_indices.size());

      mooseAssert(
          _secondary_side_phi->size() == secondary_side_dof_indices.size(),
          "The number of shapes does not match the number of dof indices on the secondary elem");

      for (_i = 0; _i < _test.size(); _i++)
        for (_secondary_j = 0;
             _secondary_j < static_cast<unsigned int>(secondary_side_dof_indices.size());
             ++_secondary_j)
          K_secondary(_i, _secondary_j) += _JxW[_qp] * _coord[_qp] * computeSecondaryQpJacobian();

      addJacobian(_subproblem.assembly(_tid, _sys.number()),
                  K_secondary,
                  _var.dofIndices(),
                  secondary_side_dof_indices,
                  _var.scalingFactor());
    }
  }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in)
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _diag_save_in.size(); i++)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

void
GapHeatTransfer::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (jvar_num == _var.number())
  {
    computeJacobian();
    return;
  }

  const auto & jvar = getVariable(jvar_num);

  prepareMatrixTag(_assembly, _var.number(), jvar_num);

  auto phi_size = jvar.dofIndices().size();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // compute this up front because it only depends on the quadrature point
    computeGapValues();

    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < phi_size; _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar_num);

    // Ok now do the contribution from the secondary side
    if (_quadrature && _has_info)
    {
      std::vector<dof_id_type> secondary_side_dof_indices;

      _sys.dofMap().dof_indices(_secondary_side, secondary_side_dof_indices, jvar_num);

      DenseMatrix<Number> K_secondary(_var.dofIndices().size(), secondary_side_dof_indices.size());

      mooseAssert(
          _secondary_side_phi->size() == secondary_side_dof_indices.size(),
          "The number of shapes does not match the number of dof indices on the secondary elem");

      for (_i = 0; _i < _test.size(); _i++)
        for (_secondary_j = 0;
             _secondary_j < static_cast<unsigned int>(secondary_side_dof_indices.size());
             ++_secondary_j)
          K_secondary(_i, _secondary_j) +=
              _JxW[_qp] * _coord[_qp] * computeSecondaryQpOffDiagJacobian(jvar_num);

      addJacobian(_subproblem.assembly(_tid, _sys.number()),
                  K_secondary,
                  _var.dofIndices(),
                  secondary_side_dof_indices,
                  _var.scalingFactor());
    }
  }

  accumulateTaggedLocalMatrix();
}

Real
GapHeatTransfer::computeQpJacobian()
{
  if (!_has_info)
    return 0.0;

  return _test[_i][_qp] *
         ((_u[_qp] - _gap_temp) * _edge_multiplier * _gap_conductance_dT[_qp] +
          _edge_multiplier * _gap_conductance[_qp]) *
         _phi[_j][_qp];
}

Real
GapHeatTransfer::computeSecondaryQpJacobian()
{
  return _test[_i][_qp] *
         ((_u[_qp] - _gap_temp) * _edge_multiplier * _gap_conductance_dT[_qp] -
          _edge_multiplier * _gap_conductance[_qp]) *
         (*_secondary_side_phi)[_secondary_j][0];
}

Real
GapHeatTransfer::computeQpOffDiagJacobian(unsigned jvar)
{
  if (!_has_info)
    return 0.0;

  unsigned int coupled_component;
  bool active = false;
  for (coupled_component = 0; coupled_component < _disp_vars.size(); ++coupled_component)
    if (jvar == _disp_vars[coupled_component])
    {
      active = true;
      break;
    }

  Real dRdx = 0.0;
  if (active)
  {
    // Compute dR/du_[xyz]
    // Residual is based on
    //   h_gap = h_conduction() + h_contact() + h_radiation();
    //   grad_t = (_u[_qp] - _gap_temp) * h_gap;
    // So we need
    //   (_u[_qp] - _gap_temp) * (dh_gap/du_[xyz]);
    // Assuming dh_contact/du_[xyz] = dh_radiation/du_[xyz] = 0,
    //   we need dh_conduction/du_[xyz]
    // Given
    //   h_conduction = gapK / gapLength, then
    //   dh_conduction/du_[xyz] = -gapK/gapLength^2 * dgapLength/du_[xyz]
    // Given
    //   gapLength = ((u_x-m_x)^2+(u_y-m_y)^2+(u_z-m_z)^2)^1/2
    // where m_[xyz] is the primary coordinate, then
    //   dGapLength/du_[xyz] =
    //   1/2*((u_x-m_x)^2+(u_y-m_y)^2+(u_z-m_z)^2)^(-1/2)*2*(u_[xyz]-m_[xyz])
    //                       = (u_[xyz]-m_[xyz])/gapLength
    // This is the normal vector.

    const Real gapL = gapLength();

    // THIS IS NOT THE NORMAL WE NEED.
    // WE NEED THE NORMAL FROM THE CONSTRAINT, THE NORMAL FROM THE
    // PRIMARY SURFACE.  HOWEVER, THIS IS TRICKY SINCE THE NORMAL
    // FROM THE PRIMARY SURFACE WAS COMPUTED FOR A POINT ASSOCIATED
    // WITH A SECONDARY NODE.  NOW WE ARE AT A SECONDARY INTEGRATION POINT.
    //
    // HOW DO WE GET THE NORMAL WE NEED?
    //
    // Until we have the normal we need,
    //   we'll hope that the one we have is close to the negative of the one we need.
    const Point & normal(_normals[_qp]);

    const Real dgap = dgapLength(-normal(coupled_component));
    dRdx = -(_u[_qp] - _gap_temp) * _edge_multiplier * _gap_conductance[_qp] *
           GapConductance::gapAttenuation(gapL, _min_gap, _min_gap_order) * dgap;
  }
  return _test[_i][_qp] * dRdx * _phi[_j][_qp];
}

Real
GapHeatTransfer::computeSecondaryQpOffDiagJacobian(unsigned jvar)
{
  if (!_has_info)
    return 0.0;

  unsigned int coupled_component;
  bool active = false;
  for (coupled_component = 0; coupled_component < _disp_vars.size(); ++coupled_component)
    if (jvar == _disp_vars[coupled_component])
    {
      active = true;
      break;
    }

  Real dRdx = 0.0;
  if (active)
  {
    const Real gapL = gapLength();

    const Point & normal(_normals[_qp]);

    const Real dgap = dgapLength(-normal(coupled_component));

    // The sign of the secondary side should presumably be opposite that of the primary side
    dRdx = (_u[_qp] - _gap_temp) * _edge_multiplier * _gap_conductance[_qp] *
           GapConductance::gapAttenuation(gapL, _min_gap, _min_gap_order) * dgap;
  }
  return _test[_i][_qp] * dRdx * (*_secondary_side_phi)[_secondary_j][0];
}

Real
GapHeatTransfer::gapLength() const
{
  if (_has_info)
    return GapConductance::gapLength(_gap_geometry_type, _radius, _r1, _r2, _max_gap);

  return 1.0;
}

Real
GapHeatTransfer::dgapLength(Real normalComponent) const
{
  const Real gap_L = gapLength();
  Real dgap = 0.0;

  if (_min_gap <= gap_L && gap_L <= _max_gap)
    dgap = normalComponent;

  return dgap;
}

void
GapHeatTransfer::computeGapValues()
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
    _pinfo = _penetration_locator->_penetration_info[qnode->id()];

    _gap_temp = 0.0;
    _gap_distance = std::numeric_limits<Real>::max();
    _has_info = false;
    _edge_multiplier = 1.0;

    if (_pinfo)
    {
      _gap_distance = _pinfo->_distance;
      _has_info = true;

      _secondary_side = _pinfo->_side;
      _secondary_side_phi = &_pinfo->_side_phi;
      _gap_temp = _variable->getValue(_secondary_side, *_secondary_side_phi);

      Real tangential_tolerance = _penetration_locator->getTangentialTolerance();
      if (tangential_tolerance != 0.0)
      {
        _edge_multiplier = 1.0 - _pinfo->_tangential_distance / tangential_tolerance;
        if (_edge_multiplier < 0.0)
          _edge_multiplier = 0.0;
      }
    }
    else
    {
      if (_warnings)
        mooseWarning("No gap value information found for node ",
                     qnode->id(),
                     " on processor ",
                     processor_id());
    }
  }

  GapConductance::computeGapRadii(
      _gap_geometry_type, _q_point[_qp], _p1, _p2, _gap_distance, _normals[_qp], _r1, _r2, _radius);
}
