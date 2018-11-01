//* This file is part of the MOOSE framework
//* https://www.mooseframework.org All rights reserved, see COPYRIGHT
//* for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT Licensed
//* under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectiveFluxCalculator.h"

#include "libmesh/string_to_enum.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("PorousFlowApp", AdvectiveFluxCalculator);

template <>
InputParameters
validParams<AdvectiveFluxCalculator>()
{
  InputParameters params = validParams<ElementLoopUserObject>();
  params.addClassDescription("Computes K_ij (a measure of advective flux from node i to node j) "
                             "and R+ and R- (which quantify amount of antidiffusion to add) in the "
                             "Kuzmin-Turek FEM-TVD multidimensional scheme");
  params.addRequiredCoupledVar("u", "The variable that is being advected");
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity vector");
  MooseEnum flux_limiter_type("MinMod VanLeer MC superbee None", "MinMod");
  params.addParam<MooseEnum>("flux_limiter_type",
                             flux_limiter_type,
                             "Type of flux limiter to use.  'None' means that no antidiffusion "
                             "will be added in the Kuzmin-Turek scheme");
  // QUERY: do we need timestep_begin (see notes in timestepSetup method) or just nonlinear?
  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_TIMESTEP_BEGIN, EXEC_NONLINEAR};
  return params;
}

AdvectiveFluxCalculator::AdvectiveFluxCalculator(const InputParameters & parameters)
  : ElementLoopUserObject(parameters),
    _velocity(getParam<RealVectorValue>("velocity")),
    _u_nodal(getVar("u", 0)),
    _u_var_num(coupled("u", 0)),
    _phi(_assembly.fePhi<Real>(_u_nodal->feType())),
    _grad_phi(_assembly.feGradPhi<Real>(_u_nodal->feType())),
    _init_k_and_compute_valence(true),
    _flux_limiter_type(getParam<MooseEnum>("flux_limiter_type").getEnum<FluxLimiterTypeEnum>()),
    _kij({}),
    _valence({})
{
  // TODO: test this
  if (!_execute_enum.contains(EXEC_TIMESTEP_BEGIN) || !_execute_enum.contains(EXEC_NONLINEAR))
    mooseError("The AdvectiveFluxCalculator UserObject " + name() +
               " execute_on parameter must include, at least, 'timestep_begin nonlinear'");
}

void
AdvectiveFluxCalculator::timestepSetup()
{
  if (_init_k_and_compute_valence)
  {
    _kij.clear();
    _valence.clear();

    // NOTE: We don't want to recompute the neighboring nodes every nonlinear iteration.  We
    // only need to do that at the beginning and when the mesh has changed because of adaptivity
    // QUERY: does this properly account for multiple processors, threading, and other things i haven't thought about?

    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    for (ConstElemRange::const_iterator el = elem_range.begin(); el != elem_range.end(); ++el)
    {
      const Elem * const elem = *el;
      if (this->hasBlocks(elem->subdomain_id()))
      {
        for (unsigned i = 0; i < elem->n_nodes(); ++i)
        {
          const dof_id_type node_i = elem->node_id(i);
	  if (_kij.find(node_i) == _kij.end())
	    _kij[node_i] = {};
          for (unsigned j = 0; j < elem->n_nodes(); ++j)
	    {
	      const dof_id_type node_j = elem->node_id(j);
	      _kij[node_i][node_j] = 0.0;
	      const std::pair<dof_id_type, dof_id_type> i_j(node_i, node_j);
	      if (_valence.find(i_j) == _valence.end())
		_valence[i_j] = 0;
	      _valence[i_j] += 1;
	    }
        }
      }
    }
    _init_k_and_compute_valence = false;
  }
}

void
AdvectiveFluxCalculator::zeroKij()
{
  for (auto & nodes : _kij)
    for (auto & neighbors: nodes.second)
      neighbors.second = 0.0;
}


void
AdvectiveFluxCalculator::meshChanged()
{
  ElementLoopUserObject::meshChanged();

  // Signal that _kij and _valence need to be rebuilt
  _init_k_and_compute_valence = true;
}

/*
Real
AdvectiveFluxCalculator::val_at_node(const Node & node) const
{
  Moose::out << "node id = " << node.unique_id() << " connected to:";
  std::vector<const Node *> neighbors;
  MeshTools::find_nodal_neighbors(_mesh, node, _nodes_to_elem_map, neighbors);
  for (const auto & n : neighbors)
    Moose::out << " " << n->unique_id();
  Moose::out << "\n";

  // This is fine:
  // return _u_nodal->getNodalValue(node);
  // This is more convoluted, but it is used below in pPlus, etc
  dof_id_type dof = node.dof_number(_u_nodal->sys().number(), _u_var_num, 0);
  const NumericVector<Real> & u = *_u_nodal->sys().currentSolution();
  return u(dof);
}
*/

Real
AdvectiveFluxCalculator::rPlus(dof_id_type node_i) const
{
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;
  const Real p = PQPlusMinus(node_i, PQPlusMinusEnum::PPlus);
  if (p == 0.0)
    // Comment after Eqn (49): if P=0 then there's no antidiffusion, so no need to remove it
    return 1.0;
  const Real q = PQPlusMinus(node_i, PQPlusMinusEnum::QPlus);
  const Real r = q / p;
  return limitFlux(1.0, r);
}

Real
AdvectiveFluxCalculator::rMinus(dof_id_type node_i) const
{
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;
  const Real p = PQPlusMinus(node_i, PQPlusMinusEnum::PMinus);
  if (p == 0.0)
    // Comment after Eqn (49): if P=0 then there's no antidiffusion, so no need to remove it
    return 1.0;
  const Real q = PQPlusMinus(node_i, PQPlusMinusEnum::QMinus);
  const Real r = q / p;
  return limitFlux(1.0, r);
}

Real
AdvectiveFluxCalculator::limitFlux(Real a, Real b) const
{
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;

  if ((a >= 0.0 && b <= 0.0) || (a <= 0.0 && b >= 0.0))
    return 0.0;
  const Real s = (a > 0.0 ? 1.0 : -1.0);

  const Real lal = std::abs(a);
  const Real lbl = std::abs(b);
  switch (_flux_limiter_type)
  {
    case FluxLimiterTypeEnum::MinMod:
      return s * std::min(lal, lbl);
    case FluxLimiterTypeEnum::VanLeer:
      return s * 2 * lal * lbl / (lal + lbl);
    case FluxLimiterTypeEnum::MC:
      return s * std::min(0.5 * std::abs(a + b), 2.0 * std::min(lal, lbl));
    case FluxLimiterTypeEnum::superbee:
      return s * std::max(std::min(2.0 * lal, lbl), std::min(lal, 2.0 * lbl));
    default:
      return 0.0;
  }
}

void
AdvectiveFluxCalculator::pre()
{
  zeroKij();
}

void
AdvectiveFluxCalculator::computeElement()
{
  /// prepare variable values and test functions
  _fe_problem.prepare(_current_elem, _tid);
  _fe_problem.reinitElem(_current_elem, _tid);

  /// compute _kij contributions from this element
  for (unsigned i = 0; i < _current_elem->n_nodes(); ++i)
  {
    const dof_id_type node_i = _current_elem->node_id(i);
    for (unsigned j = 0; j < _current_elem->n_nodes(); ++j)
    {
      const dof_id_type node_j = _current_elem->node_id(j);
      for (unsigned qp = 0; qp < _qrule->n_points(); ++qp)
        // KT Eqn (20)
        _kij[node_i][node_j] +=
            _JxW[qp] * _coord[qp] * (_grad_phi[i][qp] * _velocity) * _phi[j][qp];
    }
  }
}

Real
AdvectiveFluxCalculator::getKij(dof_id_type node_i, dof_id_type node_j) const
{
  const auto & row_find = _kij.find(node_i);
  mooseAssert(row_find != _kij.end(),
              "AdvectiveFluxCalculator UserObject " << name() << " Kij does not contain node "
                                                    << node_i);
  const std::map<dof_id_type, Real> & kij_row = row_find->second;
  const auto & entry_find = kij_row.find(node_j);
  mooseAssert(entry_find != kij_row.end(),
              "AdvectiveFluxCalculator UserObject "
                  << name() << " Kij on row " << node_i << " does not contain node "
                  << node_j);

  return entry_find->second;
}

unsigned
AdvectiveFluxCalculator::getValence(dof_id_type node_i, dof_id_type node_j) const
{
  const std::pair<dof_id_type, dof_id_type> i_j(node_i, node_j);
  const auto & entry_find = _valence.find(i_j);
  mooseAssert(entry_find != _valence.end(),
              "AdvectiveFluxCalculator UserObject " << name() << " Valence does not contain node-pair " << node_i << " to " << node_j);
  return entry_find->second;
}

Real
AdvectiveFluxCalculator::PQPlusMinus(dof_id_type node_i, const PQPlusMinusEnum pq_plus_minus) const
{
  // Find the value of u at node_i
  const Real u_i = _u_nodal->getNodalValue(_mesh.getMesh().node_ref(node_i));

  Real result = 0.0;

  // Sum over all nodes connected with node_i.
  // These nodes are found in _kij[node_i]
  const auto & row_find = _kij.find(node_i);
  mooseAssert(row_find != _kij.end(),
              "AdvectiveFluxCalculator UserObject " << name() << " Kij does not contain node "
                                                    << node_i);
  for (const auto & node_j_and_real : row_find->second)
  {
    // Find the value of u at node_j
    const dof_id_type node_j = node_j_and_real.first;
    const Real u_j = _u_nodal->getNodalValue(_mesh.getMesh().node_ref(node_j));

    // Evaluate the i-j contribution to the result
    switch (pq_plus_minus)
    {
      case PQPlusMinusEnum::PPlus:
        result += std::min(0.0, getKij(node_i, node_j)) * std::min(0.0, u_j - u_i);
        break;
      case PQPlusMinusEnum::PMinus:
        result += std::min(0.0, getKij(node_i, node_j)) * std::max(0.0, u_j - u_i);
        break;
      case PQPlusMinusEnum::QPlus:
        result += std::max(0.0, getKij(node_i, node_j)) * std::max(0.0, u_j - u_i);
        break;
      case PQPlusMinusEnum::QMinus:
        result += std::max(0.0, getKij(node_i, node_j)) * std::min(0.0, u_j - u_i);
        break;
    }
  }

  return result;
}
