//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InitialConditionTempl.h"
#include "FEProblem.h"
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/fe_interface.h"
#include "libmesh/quadrature.h"

template <typename T>
InitialConditionTempl<T>::InitialConditionTempl(const InputParameters & parameters)
  : InitialConditionBase(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _tid(getParam<THREAD_ID>("_tid")),
    _t(_fe_problem.time()),
    _var(_sys.getActualFieldVariable<T>(parameters.get<THREAD_ID>("_tid"),
                                        parameters.get<VariableName>("variable"))),
    _fe_var(dynamic_cast<MooseVariableFE<T> *>(&_var)),
    _assembly(
        _fe_problem.assembly(_tid, _var.kind() == Moose::VAR_NONLINEAR ? _var.sys().number() : 0)),
    _coord_sys(_assembly.coordSystem()),
    _current_elem(_var.currentElem()),
    _current_elem_volume(_assembly.elemVolume()),
    _current_node(nullptr),
    _qp(0),
    _fe_type(_var.feType())
{
}

template <typename T>
InitialConditionTempl<T>::~InitialConditionTempl()
{
}

template <typename T>
void
InitialConditionTempl<T>::compute()
{
  // -- NOTE ----
  // The following code is a copy from libMesh project_vector.C plus it adds some features, so we
  // can couple variable values
  // and we also do not call any callbacks, but we use our initial condition system directly.
  // ------------

  // The dimension of the current element
  _dim = _current_elem->dim();
  // The element type
  const ElemType elem_type = _current_elem->type();
  // The number of nodes on the new element
  const unsigned int n_nodes = _current_elem->n_nodes();

  // Get FE objects of the appropriate type
  // We cannot use the FE object in Assembly, since the following code is messing with the
  // quadrature rules
  // for projections and would screw it up. However, if we implement projections from one mesh to
  // another,
  // this code should use that implementation.
  std::unique_ptr<FEBaseType> fe(FEBaseType::build(_dim, _fe_type));

  // Prepare variables for projection
  std::unique_ptr<QBase> qrule(_fe_type.default_quadrature_rule(_dim));
  std::unique_ptr<QBase> qedgerule(_fe_type.default_quadrature_rule(1));
  std::unique_ptr<QBase> qsiderule(_fe_type.default_quadrature_rule(_dim - 1));

  // The values of the shape functions at the quadrature points
  _phi = &fe->get_phi();

  // The gradients of the shape functions at the quadrature points on the child element.
  _dphi = nullptr;

  _cont = fe->get_continuity();

  if (_cont == C_ONE)
  {
    const std::vector<std::vector<GradientShapeType>> & ref_dphi = fe->get_dphi();
    _dphi = &ref_dphi;
  }

  // The Jacobian * quadrature weight at the quadrature points
  _JxW = &fe->get_JxW();
  // The XYZ locations of the quadrature points
  _xyz_values = &fe->get_xyz();

  // Update the DOF indices for this element based on the current mesh
  _var.prepareIC();
  _dof_indices = _var.dofIndices();

  // The number of DOFs on the element
  const unsigned int n_dofs = _dof_indices.size();
  if (n_dofs == 0)
    return;

  // Fixed vs. free DoFs on edge/face projections
  _dof_is_fixed.clear();
  _dof_is_fixed.resize(n_dofs, false);
  _free_dof.clear();
  _free_dof.resize(n_dofs, 0);

  // Zero the interpolated values
  _Ue.resize(n_dofs);
  _Ue.zero();

  DenseVector<char> mask(n_dofs, true);

  // In general, we need a series of
  // projections to ensure a unique and continuous
  // solution.  We start by interpolating nodes, then
  // hold those fixed and project edges, then
  // hold those fixed and project faces, then
  // hold those fixed and project interiors

  // Interpolate node values first
  _current_dof = 0;

  for (_n = 0; _n != n_nodes; ++_n)
  {
    _nc = FEInterface::n_dofs_at_node(_dim, _fe_type, elem_type, _n);

    // for nodes that are in more than one subdomain, only compute the initial
    // condition once on the lowest numbered block
    auto curr_node = _current_elem->node_ptr(_n);
    const auto & block_ids = _sys.mesh().getNodeBlockIds(*curr_node);

    auto priority_block = *(block_ids.begin());
    for (auto id : block_ids)
      if (_var.hasBlocks(id))
      {
        priority_block = id;
        break;
      }

    if (!hasBlocks(priority_block) && _var.isNodal())
    {
      for (decltype(_nc) i = 0; i < _nc; ++i)
      {
        mask(_current_dof) = false;
        _current_dof++;
      }
      continue;
    }

    // FIXME: this should go through the DofMap,
    // not duplicate _dof_indices code badly!
    if (!_current_elem->is_vertex(_n))
    {
      _current_dof += _nc;
      continue;
    }

    if (_cont == DISCONTINUOUS)
      libmesh_assert(_nc == 0);
    else if (_cont == C_ZERO)
      setCZeroVertices();
    else if (_fe_type.family == HERMITE)
      setHermiteVertices();
    else if (_cont == C_ONE)
      setOtherCOneVertices();
    else if (_cont == SIDE_DISCONTINUOUS)
      continue;
    else
      libmesh_error();
  } // loop over nodes

  // From here on out we won't be sampling at nodes anymore
  _current_node = nullptr;

  // In 3D, project any edge values next
  if (_dim > 2 && _cont != DISCONTINUOUS)
    for (unsigned int e = 0; e != _current_elem->n_edges(); ++e)
    {
      FEInterface::dofs_on_edge(_current_elem, _dim, _fe_type, e, _side_dofs);

      // Some edge dofs are on nodes and already
      // fixed, others are free to calculate
      _free_dofs = 0;
      for (unsigned int i = 0; i != _side_dofs.size(); ++i)
        if (!_dof_is_fixed[_side_dofs[i]])
          _free_dof[_free_dofs++] = i;

      // There may be nothing to project
      if (!_free_dofs)
        continue;

      // Initialize FE data on the edge
      fe->attach_quadrature_rule(qedgerule.get());
      fe->edge_reinit(_current_elem, e);
      _n_qp = qedgerule->n_points();

      choleskySolve(false);
    }

  // Project any side values (edges in 2D, faces in 3D)
  if (_dim > 1 && _cont != DISCONTINUOUS)
    for (unsigned int s = 0; s != _current_elem->n_sides(); ++s)
    {
      FEInterface::dofs_on_side(_current_elem, _dim, _fe_type, s, _side_dofs);

      // Some side dofs are on nodes/edges and already
      // fixed, others are free to calculate
      _free_dofs = 0;
      for (unsigned int i = 0; i != _side_dofs.size(); ++i)
        if (!_dof_is_fixed[_side_dofs[i]])
          _free_dof[_free_dofs++] = i;

      // There may be nothing to project
      if (!_free_dofs)
        continue;

      // Initialize FE data on the side
      fe->attach_quadrature_rule(qsiderule.get());
      fe->reinit(_current_elem, s);
      _n_qp = qsiderule->n_points();

      choleskySolve(false);
    }

  // Project the interior values, finally

  // Some interior dofs are on nodes/edges/sides and
  // already fixed, others are free to calculate
  _free_dofs = 0;
  for (unsigned int i = 0; i != n_dofs; ++i)
    if (!_dof_is_fixed[i])
      _free_dof[_free_dofs++] = i;

  // There may be nothing to project
  if (_free_dofs)
  {
    // Initialize FE data
    fe->attach_quadrature_rule(qrule.get());
    fe->reinit(_current_elem);
    _n_qp = qrule->n_points();

    choleskySolve(true);
  } // if there are free interior dofs

  // Make sure every DoF got reached!
  for (unsigned int i = 0; i != n_dofs; ++i)
    libmesh_assert(_dof_is_fixed[i]);

  // Lock the new_vector since it is shared among threads.
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (size_t i = 0; i < mask.size(); i++)
      if (mask(i))
        _var.setDofValue(_Ue(i), i);
  }
}

template <typename T>
void
InitialConditionTempl<T>::setCZeroVertices()
{
  // Assume that C_ZERO elements have a single nodal
  // value shape function
  libmesh_assert(_nc == 1);
  _qp = _n;
  _current_node = _current_elem->node_ptr(_n);
  _Ue(_current_dof) = value(*_current_node);
  _dof_is_fixed[_current_dof] = true;
  _current_dof++;
}

template <>
void
InitialConditionTempl<RealVectorValue>::setCZeroVertices()
{
  _qp = _n;
  _current_node = _current_elem->node_ptr(_n);
  auto point_value = value(*_current_node);
  for (decltype(_nc) i = 0; i < _nc; ++i)
  {
    _Ue(_current_dof) = point_value(i);
    _dof_is_fixed[_current_dof] = true;
    _current_dof++;
  }
}

template <typename T>
T
InitialConditionTempl<T>::gradientComponent(GradientType grad, unsigned int i)
{
  return grad(i);
}

template <>
RealVectorValue
InitialConditionTempl<RealVectorValue>::gradientComponent(GradientType grad, unsigned int i)
{
  return grad.row(i);
}

template <>
RealEigenVector
InitialConditionTempl<RealEigenVector>::gradientComponent(GradientType grad, unsigned int i)
{
  return grad.col(i);
}

template <typename T>
void
InitialConditionTempl<T>::setHermiteVertices()
{
  // The hermite element vertex shape functions are weird
  _qp = _n;
  _current_node = _current_elem->node_ptr(_n);
  _Ue(_current_dof) = value(*_current_node);
  _dof_is_fixed[_current_dof] = true;
  _current_dof++;
  GradientType grad = gradient(*_current_node);
  // x derivative
  _Ue(_current_dof) = gradientComponent(grad, 0);
  _dof_is_fixed[_current_dof] = true;
  _current_dof++;
  if (_dim > 1)
  {
    // We'll finite difference mixed derivatives
    Point nxminus = _current_elem->point(_n), nxplus = _current_elem->point(_n);
    nxminus(0) -= TOLERANCE;
    nxplus(0) += TOLERANCE;
    GradientType gxminus = gradient(nxminus);
    GradientType gxplus = gradient(nxplus);
    // y derivative
    _Ue(_current_dof) = gradientComponent(grad, 1);
    _dof_is_fixed[_current_dof] = true;
    _current_dof++;
    // xy derivative
    _Ue(_current_dof) =
        (gradientComponent(gxplus, 1) - gradientComponent(gxminus, 1)) / 2. / TOLERANCE;
    _dof_is_fixed[_current_dof] = true;
    _current_dof++;

    if (_dim > 2)
    {
      // z derivative
      _Ue(_current_dof) = gradientComponent(grad, 2);
      _dof_is_fixed[_current_dof] = true;
      _current_dof++;
      // xz derivative
      _Ue(_current_dof) =
          (gradientComponent(gxplus, 2) - gradientComponent(gxminus, 2)) / 2. / TOLERANCE;
      _dof_is_fixed[_current_dof] = true;
      _current_dof++;
      // We need new points for yz
      Point nyminus = _current_elem->point(_n), nyplus = _current_elem->point(_n);
      nyminus(1) -= TOLERANCE;
      nyplus(1) += TOLERANCE;
      GradientType gyminus = gradient(nyminus);
      GradientType gyplus = gradient(nyplus);
      // xz derivative
      _Ue(_current_dof) =
          (gradientComponent(gyplus, 2) - gradientComponent(gyminus, 2)) / 2. / TOLERANCE;
      _dof_is_fixed[_current_dof] = true;
      _current_dof++;
      // Getting a 2nd order xyz is more tedious
      Point nxmym = _current_elem->point(_n), nxmyp = _current_elem->point(_n),
            nxpym = _current_elem->point(_n), nxpyp = _current_elem->point(_n);
      nxmym(0) -= TOLERANCE;
      nxmym(1) -= TOLERANCE;
      nxmyp(0) -= TOLERANCE;
      nxmyp(1) += TOLERANCE;
      nxpym(0) += TOLERANCE;
      nxpym(1) -= TOLERANCE;
      nxpyp(0) += TOLERANCE;
      nxpyp(1) += TOLERANCE;
      GradientType gxmym = gradient(nxmym);
      GradientType gxmyp = gradient(nxmyp);
      GradientType gxpym = gradient(nxpym);
      GradientType gxpyp = gradient(nxpyp);
      DataType gxzplus =
          (gradientComponent(gxpyp, 2) - gradientComponent(gxmyp, 2)) / 2. / TOLERANCE;
      DataType gxzminus =
          (gradientComponent(gxpym, 2) - gradientComponent(gxmym, 2)) / 2. / TOLERANCE;
      // xyz derivative
      _Ue(_current_dof) = (gxzplus - gxzminus) / 2. / TOLERANCE;
      _dof_is_fixed[_current_dof] = true;
      _current_dof++;
    }
  }
}

template <>
void
InitialConditionTempl<RealVectorValue>::setHermiteVertices()
{
}

template <typename T>
void
InitialConditionTempl<T>::setOtherCOneVertices()
{
  // Assume that other C_ONE elements have a single nodal
  // value shape function and nodal gradient component
  // shape functions
  libmesh_assert(_nc == 1 + _dim);
  _current_node = _current_elem->node_ptr(_n);
  _Ue(_current_dof) = value(*_current_node);
  _dof_is_fixed[_current_dof] = true;
  _current_dof++;
  GradientType grad = gradient(*_current_node);
  for (unsigned int i = 0; i != _dim; ++i)
  {
    _Ue(_current_dof) = gradientComponent(grad, i);
    _dof_is_fixed[_current_dof] = true;
    _current_dof++;
  }
}

template <>
void
InitialConditionTempl<RealVectorValue>::setOtherCOneVertices()
{
}

template <typename T>
void
InitialConditionTempl<T>::choleskyAssembly(bool is_volume)
{
  // Loop over the quadrature points
  for (_qp = 0; _qp < _n_qp; _qp++)
  {
    // solution at the quadrature point
    auto fineval = value((*_xyz_values)[_qp]);
    // solution grad at the quadrature point
    GradientType finegrad;
    if (_cont == C_ONE)
      finegrad = gradient((*_xyz_values)[_qp]);

    auto dofs_size = is_volume ? _dof_indices.size() : _side_dofs.size();

    // Form edge projection matrix
    for (decltype(dofs_size) geomi = 0, freei = 0; geomi != dofs_size; ++geomi)
    {
      auto i = is_volume ? geomi : _side_dofs[geomi];

      // fixed DoFs aren't test functions
      if (_dof_is_fixed[i])
        continue;
      for (decltype(dofs_size) geomj = 0, freej = 0; geomj != dofs_size; ++geomj)
      {
        auto j = is_volume ? geomj : _side_dofs[geomj];
        if (_dof_is_fixed[j])
          _Fe(freei) -= (*_phi)[i][_qp] * (*_phi)[j][_qp] * (*_JxW)[_qp] * _Ue(j);
        else
          _Ke(freei, freej) += (*_phi)[i][_qp] * (*_phi)[j][_qp] * (*_JxW)[_qp];
        if (_cont == C_ONE)
        {
          if (_dof_is_fixed[j])
            _Fe(freei) -= dotHelper((*_dphi)[i][_qp], (*_dphi)[j][_qp]) * (*_JxW)[_qp] * _Ue(j);
          else
            _Ke(freei, freej) += dotHelper((*_dphi)[i][_qp], (*_dphi)[j][_qp]) * (*_JxW)[_qp];
        }
        if (!_dof_is_fixed[j])
          freej++;
      }
      _Fe(freei) += (*_phi)[i][_qp] * fineval * (*_JxW)[_qp];
      if (_cont == C_ONE)
        _Fe(freei) += dotHelper(finegrad, (*_dphi)[i][_qp]) * (*_JxW)[_qp];
      freei++;
    }
  }
}

template <typename T>
void
InitialConditionTempl<T>::choleskySolve(bool is_volume)
{
  _Ke.resize(_free_dofs, _free_dofs);
  _Ke.zero();
  _Fe.resize(_free_dofs);
  _Fe.zero();

  choleskyAssembly(is_volume);

  // The new edge coefficients
  DenseVector<DataType> U(_free_dofs);

  _Ke.cholesky_solve(_Fe, U);

  // Transfer new edge solutions to element
  for (unsigned int i = 0; i != _free_dofs; ++i)
  {
    auto the_dof = is_volume ? _free_dof[i] : _side_dofs[_free_dof[i]];
    DataType & ui = _Ue(the_dof);
    libmesh_assert(std::abs(ui) < TOLERANCE || std::abs(ui - U(i)) < TOLERANCE);
    ui = U(i);
    _dof_is_fixed[the_dof] = true;
  }
}

template <>
void
InitialConditionTempl<RealEigenVector>::choleskySolve(bool is_volume)
{
  _Ke.resize(_free_dofs, _free_dofs);
  _Ke.zero();
  _Fe.resize(_free_dofs);
  for (unsigned int i = 0; i < _free_dofs; ++i)
    _Fe(i).setZero(_var.count());

  choleskyAssembly(is_volume);

  // The new edge coefficients
  DenseVector<DataType> U = _Fe;

  for (unsigned int i = 0; i < _var.count(); ++i)
  {
    DenseVector<Real> v(_free_dofs), x(_free_dofs);
    for (unsigned int j = 0; j < _free_dofs; ++j)
      v(j) = _Fe(j)(i);

    _Ke.cholesky_solve(v, x);

    for (unsigned int j = 0; j < _free_dofs; ++j)
      U(j)(i) = x(j);
  }

  // Transfer new edge solutions to element
  for (unsigned int i = 0; i != _free_dofs; ++i)
  {
    auto the_dof = is_volume ? _free_dof[i] : _side_dofs[_free_dof[i]];
    DataType & ui = _Ue(the_dof);
    libmesh_assert(ui.matrix().norm() < TOLERANCE || (ui - U(i)).matrix().norm() < TOLERANCE);
    ui = U(i);
    _dof_is_fixed[the_dof] = true;
  }
}

template <typename T>
void
InitialConditionTempl<T>::computeNodal(const Point & p)
{
  _var.reinitNode();
  _var.computeNodalValues(); // has to call this to resize the internal array
  auto return_value = value(p);

  _var.setNodalValue(return_value); // update variable data, which is referenced by others, so the
                                    // value is up-to-date

  // We are done, so update the solution vector
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _var.insert(_var.sys().solution());
  }
}

template class InitialConditionTempl<Real>;
template class InitialConditionTempl<RealVectorValue>;
template class InitialConditionTempl<RealEigenVector>;
