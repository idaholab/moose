//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrossFieldSolver.h"

#include "MooseError.h"

#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/enum_elem_type.h"
#include "libmesh/enum_fe_family.h"
#include "libmesh/enum_order.h"
#include "libmesh/enum_point_locator_type.h"
#include "libmesh/enum_to_string.h"
#include "libmesh/fe_base.h"
#include "libmesh/fe_compute_data.h"
#include "libmesh/fe_interface.h"
#include "libmesh/fe_map.h"
#include "libmesh/int_range.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/linear_solver.h"
#include "libmesh/node.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/utility.h"

#include <algorithm>
#include <cmath>

using namespace libMesh;

namespace
{
/// Penalty weight of the nodal Dirichlet conditions, large enough to pin the boundary values of z
/// to the relative tolerance of the linear solve.
constexpr Real dirichlet_penalty = 1.0e10;

/// Relative tolerance of the linear solve, tight enough that the generated mesh is reproducible.
constexpr Real linear_solver_tolerance = 1.0e-10;

/// Magnitude of z, measured before normalization, below which a node is a field singularity.
constexpr Real singular_magnitude = 0.1;

/// Magnitude of z below which normalization is abandoned rather than dividing by zero.
constexpr Real minimum_magnitude = 1.0e-12;

CrossFieldSolver::NodalCrossField
boundaryCrossField(const std::map<dof_id_type, Real> & boundary_tangent_angles)
{
  CrossFieldSolver::NodalCrossField boundary_cross_field;
  for (const auto & [node_id, tangent_angle] : boundary_tangent_angles)
    boundary_cross_field.emplace(node_id, std::polar(1.0, 4.0 * tangent_angle));
  return boundary_cross_field;
}
}

CrossFieldSolver::CrossFieldSolver(const MeshBase & background_mesh,
                                   const std::map<dof_id_type, Real> & boundary_tangent_angles)
  : _communicator(MPI_COMM_SELF),
    _mesh(_communicator),
    _boundary_cross_field(boundaryCrossField(boundary_tangent_angles)),
    _fe_type(FIRST, LAGRANGE),
    _system(nullptr),
    _real_variable(0),
    _imaginary_variable(0),
    _solved(false)
{
  // Only the local portion of a distributed mesh would be copied, which would silently solve on a
  // fragment of the domain.
  if (!background_mesh.is_replicated())
    mooseError("CrossFieldSolver: The cross field background mesh must be replicated.");

  // Node ids key the boundary data and every reported result, so they have to survive the copy onto
  // the serial communicator.
  _mesh.allow_renumbering(false);
  _mesh.copy_nodes_and_elements(background_mesh);
  _mesh.prepare_for_use();

  for (const auto & elem : _mesh.element_ptr_range())
    if (elem->type() != TRI3)
      mooseError("CrossFieldSolver: The cross field background mesh must contain only TRI3 "
                 "elements, but it contains an element of type ",
                 Utility::enum_to_string(elem->type()),
                 ".");

  for (const auto & [node_id, _] : _boundary_cross_field)
    if (!_mesh.query_node_ptr(node_id))
      mooseError("CrossFieldSolver: A boundary tangent angle was supplied for node ",
                 node_id,
                 ", which is not a node of the cross field background mesh.");

  _equation_systems = std::make_unique<EquationSystems>(_mesh);
  _system = &_equation_systems->add_system<LinearImplicitSystem>("cross_field");
  _real_variable = _system->add_variable("z_real", _fe_type);
  _imaginary_variable = _system->add_variable("z_imaginary", _fe_type);
  _system->attach_assemble_function(assembleSystem);
  _equation_systems->parameters.set<CrossFieldSolver *>("cross_field_solver") = this;
  _equation_systems->init();

  _point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, _mesh);
  _point_locator->enable_out_of_mesh_mode();
}

void
CrossFieldSolver::solve()
{
  _equation_systems->parameters.set<Real>("linear solver tolerance") = linear_solver_tolerance;
  _system->solve();

  const auto reason = _system->get_linear_solver()->get_converged_reason();
  if (reason < 0)
    mooseError("CrossFieldSolver: The cross field solve failed to converge with reason: ",
               Utility::enum_to_string(reason));

  extractNodalCrossField();
  _solved = true;
}

void
CrossFieldSolver::assembleSystem(EquationSystems & es, const std::string & /*system_name*/)
{
  es.parameters.get<CrossFieldSolver *>("cross_field_solver")->assembleLaplace();
}

void
CrossFieldSolver::assembleLaplace()
{
  const DofMap & dof_map = _system->get_dof_map();
  std::unique_ptr<FEBase> fe(FEBase::build(_mesh.mesh_dimension(), _fe_type));
  QGauss qrule(_mesh.mesh_dimension(), _fe_type.default_quadrature_order());
  fe->attach_quadrature_rule(&qrule);

  const std::vector<Real> & JxW = fe->get_JxW();
  const std::vector<std::vector<RealGradient>> & dphi = fe->get_dphi();

  DenseMatrix<Number> Ke;
  DenseVector<Number> Fe_real;
  DenseVector<Number> Fe_imaginary;
  std::vector<dof_id_type> real_dof_indices;
  std::vector<dof_id_type> imaginary_dof_indices;

  auto & system_matrix = _system->get_system_matrix();

  for (const auto & elem : _mesh.active_local_element_ptr_range())
  {
    fe->reinit(elem);
    dof_map.dof_indices(elem, real_dof_indices, _real_variable);
    dof_map.dof_indices(elem, imaginary_dof_indices, _imaginary_variable);

    const auto n_dofs = real_dof_indices.size();
    Ke.resize(n_dofs, n_dofs);
    Fe_real.resize(n_dofs);
    Fe_imaginary.resize(n_dofs);

    for (const auto qp : make_range(qrule.n_points()))
      for (const auto i : make_range(n_dofs))
        for (const auto j : make_range(n_dofs))
          Ke(i, j) += JxW[qp] * (dphi[i][qp] * dphi[j][qp]);

    // The Dirichlet data is given per node, so the boundary condition is a penalty on the node's
    // own equation rather than a side integral. A node shared by several elements accumulates the
    // penalty once per element on both the diagonal and the right hand side, which leaves the value
    // it pins unchanged.
    mooseAssert(n_dofs == elem->n_nodes(),
                "The nodal penalty indexes degrees of freedom by local node number, which linear "
                "Lagrange on TRI3 must supply one of per element node.");
    for (const auto i : make_range(n_dofs))
    {
      const auto boundary_value = _boundary_cross_field.find(elem->node_id(i));
      if (boundary_value == _boundary_cross_field.end())
        continue;

      Ke(i, i) += dirichlet_penalty;
      Fe_real(i) += dirichlet_penalty * boundary_value->second.real();
      Fe_imaginary(i) += dirichlet_penalty * boundary_value->second.imag();
    }

    // Both components of z obey the same Laplace operator and couple only through the boundary
    // data, so one element matrix serves both variables.
    system_matrix.add_matrix(Ke, real_dof_indices);
    system_matrix.add_matrix(Ke, imaginary_dof_indices);
    _system->rhs->add_vector(Fe_real, real_dof_indices);
    _system->rhs->add_vector(Fe_imaginary, imaginary_dof_indices);
  }
}

void
CrossFieldSolver::extractNodalCrossField()
{
  const auto system_number = _system->number();
  const auto & solution = *_system->solution;

  _nodal_cross_field.clear();
  _singular_nodes.clear();

  for (const auto & node : _mesh.node_ptr_range())
  {
    // The system lives on a serial communicator, so every degree of freedom is local here.
    const std::complex<Real> z(solution(node->dof_number(system_number, _real_variable, 0)),
                               solution(node->dof_number(system_number, _imaginary_variable, 0)));
    const Real magnitude = std::abs(z);

    if (magnitude < singular_magnitude)
      _singular_nodes.push_back(node->id());

    // A vanishing z carries no direction at all, so fall back on the real axis instead of dividing
    // by zero. Such a node is always singular by the test above.
    const auto normalized =
        magnitude > minimum_magnitude ? z / magnitude : std::complex<Real>(1.0, 0.0);
    _nodal_cross_field.emplace(node->id(), normalized);
  }

  std::sort(_singular_nodes.begin(), _singular_nodes.end());
}

std::complex<Real>
CrossFieldSolver::interpolatedCrossField(const Point & point) const
{
  mooseAssert(_solved, "solve() must be called before the cross field can be queried.");

  const Elem * elem = (*_point_locator)(point);
  if (!elem)
    mooseError("CrossFieldSolver: No element was found to contain point ", point);

  const Point reference_point = FEMap::inverse_map(elem->dim(), elem, point);
  FEComputeData fe_data(*_equation_systems, reference_point);
  FEInterface::compute_data(elem->dim(), _fe_type, elem, fe_data);

  mooseAssert(fe_data.shape.size() == elem->n_nodes(),
              "Linear Lagrange on TRI3 must supply one shape function per element node.");

  // Interpolating z rather than theta is what makes this well posed: theta is only defined modulo
  // pi/2, so averaging nodal angles is wrong wherever the branch cut falls between the nodes.
  std::complex<Real> interpolated(0.0, 0.0);
  for (const auto i : index_range(fe_data.shape))
  {
    const Real shape_value = fe_data.shape[i];
    interpolated += shape_value * libmesh_map_find(_nodal_cross_field, elem->node_id(i));
  }

  return interpolated;
}

Real
CrossFieldSolver::theta(const Point & point) const
{
  return std::arg(interpolatedCrossField(point)) / 4.0;
}

std::pair<Point, Point>
CrossFieldSolver::crossFrame(const Point & point) const
{
  const Real angle = theta(point);
  const Point u(std::cos(angle), std::sin(angle), 0.0);
  return {u, Point(-u(1), u(0), 0.0)};
}
