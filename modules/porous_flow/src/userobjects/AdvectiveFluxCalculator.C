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
    // QUERY: does this properly account for multiple processors, threading, and other things i
    // haven't thought about?

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
    for (auto & neighbors : nodes.second)
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
AdvectiveFluxCalculator::rPlus(dof_id_type node_i, std::map<dof_id_type, Real> & dlimited_du) const
{
  dlimited_du.clear();
  dlimited_du = zeroedConnection(node_i);
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;
  std::map<dof_id_type, Real> dp_du;
  const Real p = PQPlusMinus(node_i, PQPlusMinusEnum::PPlus, dp_du);
  if (p == 0.0)
    // Comment after Eqn (49): if P=0 then there's no antidiffusion, so no need to remove it
    return 1.0;
  std::map<dof_id_type, Real> dq_du;
  const Real q = PQPlusMinus(node_i, PQPlusMinusEnum::QPlus, dq_du);
  const Real r = q / p;
  std::map<dof_id_type, Real> dr_du = {};
  for (const auto & node_deriv : dp_du)
    dr_du[node_deriv.first] = dq_du[node_deriv.first] / p - q * node_deriv.second / std::pow(p, 2);
  Real limited;
  Real dlimited_dr;
  limitFlux(1.0, r, limited, dlimited_dr);
  for (const auto & node_deriv : dr_du)
    dlimited_du[node_deriv.first] = dlimited_dr * node_deriv.second;
  return limited;
}

Real
AdvectiveFluxCalculator::rMinus(dof_id_type node_i, std::map<dof_id_type, Real> & dlimited_du) const
{
  dlimited_du.clear();
  dlimited_du = zeroedConnection(node_i);
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;
  std::map<dof_id_type, Real> dp_du;
  const Real p = PQPlusMinus(node_i, PQPlusMinusEnum::PMinus, dp_du);
  if (p == 0.0)
    // Comment after Eqn (49): if P=0 then there's no antidiffusion, so no need to remove it
    return 1.0;
  std::map<dof_id_type, Real> dq_du;
  const Real q = PQPlusMinus(node_i, PQPlusMinusEnum::QMinus, dq_du);
  const Real r = q / p;
  std::map<dof_id_type, Real> dr_du = {};
  for (const auto & node_deriv : dp_du)
    dr_du[node_deriv.first] = dq_du[node_deriv.first] / p - q * node_deriv.second / std::pow(p, 2);
  Real limited;
  Real dlimited_dr;
  limitFlux(1.0, r, limited, dlimited_dr);
  for (const auto & node_deriv : dr_du)
    dlimited_du[node_deriv.first] = dlimited_dr * node_deriv.second;
  return limited;
}

void
AdvectiveFluxCalculator::limitFlux(Real a, Real b, Real & limited, Real & dlimited_db) const
{
  limited = 0.0;
  dlimited_db = 0.0;
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return;

  if ((a >= 0.0 && b <= 0.0) || (a <= 0.0 && b >= 0.0))
    return;
  const Real s = (a > 0.0 ? 1.0 : -1.0);

  const Real lal = std::abs(a);
  const Real lbl = std::abs(b);
  const Real dlbl = (b >= 0.0 ? 1.0 : -1.0); // d(lbl)/db
  switch (_flux_limiter_type)
  {
    case FluxLimiterTypeEnum::MinMod:
    {
      if (lal <= lbl)
      {
        limited = s * lal;
        dlimited_db = 0.0;
      }
      else
      {
        limited = s * lbl;
        dlimited_db = s * dlbl;
      }
      return;
    }
    case FluxLimiterTypeEnum::VanLeer:
    {
      limited = s * 2 * lal * lbl / (lal + lbl);
      dlimited_db = s * 2 * lal * (dlbl / (lal + lbl) - lbl * dlbl / std::pow(lal + lbl, 2));
      return;
    }
    case FluxLimiterTypeEnum::MC:
    {
      const Real av = 0.5 * std::abs(a + b);
      if (2 * lal <= av && lal <= lbl)
      {
        // 2 * lal is the smallest
        limited = s * 2.0 * lal;
        dlimited_db = 0.0;
      }
      else if (2 * lbl <= av && lbl <= lal)
      {
        // 2 * lbl is the smallest
        limited = s * 2.0 * lbl;
        dlimited_db = s * 2.0 * dlbl;
      }
      else
      {
        // av is the smallest
        limited = s * av;
        // if (a>0 and b>0) then d(av)/db = 0.5 = 0.5 * dlbl
        // if (a<0 and b<0) then d(av)/db = -0.5 = 0.5 * dlbl
        // if a and b have different sign then limited=0, above
        dlimited_db = s * 0.5 * dlbl;
      }
      return;
    }
    case FluxLimiterTypeEnum::superbee:
    {
      const Real term1 = std::min(2.0 * lal, lbl);
      const Real term2 = std::min(lal, 2.0 * lbl);
      if (term1 >= term2)
      {
        if (2.0 * lal <= lbl)
        {
          limited = s * 2 * lal;
          dlimited_db = 0.0;
        }
        else
        {
          limited = s * lbl;
          dlimited_db = s * dlbl;
        }
      }
      else
      {
        if (lal <= 2.0 * lbl)
        {
          limited = s * lal;
          dlimited_db = 0.0;
        }
        else
        {
          limited = s * 2.0 * lbl;
          dlimited_db = s * 2.0 * dlbl;
        }
      }
      return;
    }
    default:
      return;
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
              "AdvectiveFluxCalculator UserObject " << name() << " Kij on row " << node_i
                                                    << " does not contain node " << node_j);

  return entry_find->second;
}

unsigned
AdvectiveFluxCalculator::getValence(dof_id_type node_i, dof_id_type node_j) const
{
  const std::pair<dof_id_type, dof_id_type> i_j(node_i, node_j);
  const auto & entry_find = _valence.find(i_j);
  mooseAssert(entry_find != _valence.end(),
              "AdvectiveFluxCalculator UserObject "
                  << name() << " Valence does not contain node-pair " << node_i << " to "
                  << node_j);
  return entry_find->second;
}

std::map<dof_id_type, Real>
AdvectiveFluxCalculator::zeroedConnection(dof_id_type node_i) const
{
  const auto & row_find = _kij.find(node_i);
  mooseAssert(row_find != _kij.end(),
              "AdvectiveFluxCalculator UserObject " << name() << " Kij does not contain node "
                                                    << node_i);
  std::map<dof_id_type, Real> result = {};
  for (const auto & nk : row_find->second)
    result[nk.first] = 0.0;
  return result;
}

Real
AdvectiveFluxCalculator::PQPlusMinus(dof_id_type node_i,
                                     const PQPlusMinusEnum pq_plus_minus,
                                     std::map<dof_id_type, Real> & derivs) const
{
  // Find the value of u at node_i
  const Real u_i = _u_nodal->getNodalValue(_mesh.getMesh().node_ref(node_i));

  // We're going to sum over all nodes connected with node_i
  // These nodes are found in _kij[node_i]
  const auto & row_find = _kij.find(node_i);
  mooseAssert(row_find != _kij.end(),
              "AdvectiveFluxCalculator UserObject " << name() << " Kij does not contain node "
                                                    << node_i);
  const std::map<dof_id_type, Real> nodej_and_kij = row_find->second;

  // Initialize the results
  Real result = 0.0;
  derivs.clear();
  derivs = zeroedConnection(node_i);

  // Sum over all nodes connected with node_i.
  for (const auto & nk : nodej_and_kij)
  {
    const dof_id_type node_j = nk.first;
    const Real kentry = nk.second;

    // Find the value of u at node_j
    const Real u_j = _u_nodal->getNodalValue(_mesh.getMesh().node_ref(node_j));
    const Real ujminusi = u_j - u_i;

    // Evaluate the i-j contribution to the result
    switch (pq_plus_minus)
    {
      case PQPlusMinusEnum::PPlus:
      {
        if (ujminusi < 0.0)
        {
          const Real prefactor = std::min(0.0, kentry);
          result += prefactor * ujminusi;
          derivs[node_j] += prefactor;
          derivs[node_i] -= prefactor;
        }
        break;
      }
      case PQPlusMinusEnum::PMinus:
      {
        if (ujminusi > 0.0)
        {
          const Real prefactor = std::min(0.0, kentry);
          result += prefactor * ujminusi;
          derivs[node_j] += prefactor;
          derivs[node_i] -= prefactor;
        }
        break;
      }
      case PQPlusMinusEnum::QPlus:
      {
        if (ujminusi > 0.0)
        {
          const Real prefactor = std::max(0.0, kentry);
          result += prefactor * ujminusi;
          derivs[node_j] += prefactor;
          derivs[node_i] -= prefactor;
        }
        break;
      }
      case PQPlusMinusEnum::QMinus:
      {
        if (ujminusi < 0.0)
        {
          const Real prefactor = std::max(0.0, kentry);
          result += prefactor * ujminusi;
          derivs[node_j] += prefactor;
          derivs[node_i] -= prefactor;
        }
        break;
      }
    }
  }

  return result;
}
