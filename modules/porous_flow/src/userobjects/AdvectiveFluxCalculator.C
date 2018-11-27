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
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription("Computes K_ij (a measure of advective flux from node i to node j) "
                             "and R+ and R- (which quantify amount of antidiffusion to add) in the "
                             "Kuzmin-Turek FEM-TVD multidimensional scheme");
  params.addRequiredCoupledVar("u", "The variable that is being advected");
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity vector");
  MooseEnum flux_limiter_type("MinMod VanLeer MC superbee None", "VanLeer");
  params.addParam<MooseEnum>("flux_limiter_type",
                             flux_limiter_type,
                             "Type of flux limiter to use.  'None' means that no antidiffusion "
                             "will be added in the Kuzmin-Turek scheme");
  params.registerRelationshipManagers("ElementSideNeighborLayers");
  params.addPrivateParam<unsigned short>("element_side_neighbor_layers", 2);
  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_LINEAR};
  return params;
}

AdvectiveFluxCalculator::AdvectiveFluxCalculator(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _velocity(getParam<RealVectorValue>("velocity")),
    _u_nodal(getVar("u", 0)),
    _u_var_num(coupled("u", 0)),
    _phi(_assembly.fePhi<Real>(_u_nodal->feType())),
    _grad_phi(_assembly.feGradPhi<Real>(_u_nodal->feType())),
    _resizing_needed(true),
    _flux_limiter_type(getParam<MooseEnum>("flux_limiter_type").getEnum<FluxLimiterTypeEnum>()),
    _kij({}),
    _flux_out({}),
    _dflux_out_du({}),
    _valence({})
{
  if (!_execute_enum.contains(EXEC_LINEAR))
    paramError(
        "execute_on",
        "The AdvectiveFluxCalculator UserObject " + name() +
            " execute_on parameter must include, at least, 'linear'.  This is to ensure that "
            "this UserObject computes all necessary quantities just before the Kernels evaluate "
            "their Residuals");
}

void
AdvectiveFluxCalculator::timestepSetup()
{
  if (_resizing_needed)
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

    _flux_out.clear();
    _dflux_out_du.clear();
    for (auto & nodes : _kij)
    {
      const dof_id_type node_i = nodes.first;
      _flux_out[node_i] = 0.0;
      _dflux_out_du[node_i] = {};
      for (auto & neighbors : nodes.second)
      {
        const dof_id_type node_j = neighbors.second;
        _dflux_out_du[node_i][node_j] = 0.0;
      }
    }
    _resizing_needed = false;
  }
}

void
AdvectiveFluxCalculator::meshChanged()
{
  ElementUserObject::meshChanged();

  // Signal that _kij and _valence need to be rebuilt
  _resizing_needed = true;
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
AdvectiveFluxCalculator::initialize()
{
  for (auto & nodes : _kij)
    for (auto & neighbors : nodes.second)
      neighbors.second = 0.0;
}

void
AdvectiveFluxCalculator::execute()
{
  /// prepare variable values and test functions
  //_fe_problem.prepare(_current_elem, _tid);
  //_fe_problem.reinitElem(_current_elem, _tid);

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

std::map<dof_id_type, Real>
AdvectiveFluxCalculator::getdFluxOutdu(dof_id_type node_i) const
{
  const auto & row_find = _dflux_out_du.find(node_i);
  mooseAssert(row_find != _dflux_out_du.end(),
              "AdvectiveFluxCalculator UserObject "
                  << name() << " _dflux_out_du does not contain node " << node_i);
  return row_find->second;
}

Real
AdvectiveFluxCalculator::getdFluxOutdu(dof_id_type node_i, dof_id_type node_j) const
{
  const auto & row_find = _dflux_out_du.find(node_i);
  mooseAssert(row_find != _dflux_out_du.end(),
              "AdvectiveFluxCalculator UserObject "
                  << name() << " _dflux_out_du does not contain node " << node_i);
  const std::map<dof_id_type, Real> & dflux_out_du_row = row_find->second;
  const auto & entry_find = dflux_out_du_row.find(node_j);
  mooseAssert(entry_find != dflux_out_du_row.end(),
              "AdvectiveFluxCalculator UserObject " << name() << " _dflux_out_du on row " << node_i
                                                    << " does not contain node " << node_j);

  return entry_find->second;
}

Real
AdvectiveFluxCalculator::getFluxOut(dof_id_type node_i) const
{
  const auto & entry_find = _flux_out.find(node_i);
  mooseAssert(entry_find != _flux_out.end(),
              "AdvectiveFluxCalculator UserObject " << name() << " _flux_out does not contain node "
                                                    << node_i);
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

void
AdvectiveFluxCalculator::threadJoin(const UserObject & uo)
{
  const AdvectiveFluxCalculator & afc = static_cast<const AdvectiveFluxCalculator &>(uo);
  // because afc is const, loop through afc._kij rather than through _kij
  for (const auto & nodes : afc._kij)
  {
    const dof_id_type i = nodes.first;
    for (const auto & neighbors : nodes.second)
    {
      const dof_id_type j = neighbors.first;
      const Real afc_val = neighbors.second;
      _kij[i][j] += afc_val;
    }
  }
}

void
AdvectiveFluxCalculator::finalize()
{
  // Calculate KuzminTurek D matrix
  // See Eqn (32)
  // This adds artificial diffusion, which eliminates any spurious oscillations
  // The idea is that D will remove all negative off-diagonal elements when it is added to K
  // This is identical to full upwinding
  std::map<dof_id_type, std::map<dof_id_type, Real>> dij = {};
  for (auto & nodes : _kij)
  {
    const dof_id_type i = nodes.first;
    dij[i] = {};
    dij[i][i] = 0.0;
    for (auto & neighbors : nodes.second)
    {
      const dof_id_type j = neighbors.first;
      if (i == j)
        continue;
      dij[i][j] = std::max(0.0, std::max(-_kij[i][j], -_kij[j][i]));
      dij[i][i] -= dij[i][j];
    }
  }

  // Calculate KuzminTurek L matrix
  // See Fig 2: L = K + D
  std::map<dof_id_type, std::map<dof_id_type, Real>> lij = {};
  for (auto & nodes : _kij)
  {
    const dof_id_type i = nodes.first;
    lij[i] = {};
    for (auto & neighbors : nodes.second)
    {
      const dof_id_type j = neighbors.first;
      lij[i][j] = _kij[i][j] + dij[i][j];
    }
  }

  // Compute KuzminTurek R matrices
  // See Eqns (49) and (12)
  std::map<dof_id_type, Real> rP = {};
  std::map<dof_id_type, Real> rM = {};
  std::map<dof_id_type, std::map<dof_id_type, Real>> drP = {};
  std::map<dof_id_type, std::map<dof_id_type, Real>> drM = {};
  for (const auto & nodes : _kij)
  {
    const dof_id_type i = nodes.first;
    drP[i] = {};
    rP[i] = rPlus(i, drP[i]);
    drM[i] = {};
    rM[i] = rMinus(i, drM[i]);
  }

  // Calculate KuzminTurek f^{a} matrix
  // This is the antidiffusive flux
  // See Eqn (50)
  std::map<dof_id_type, std::map<dof_id_type, Real>> fa = {};
  std::map<dof_id_type, std::map<dof_id_type, std::map<dof_id_type, Real>>> dfa =
      {}; // dfa[i][j][k] = d(fa[i][j])/du[k]
  for (const auto & nodes : _kij)
  {
    const dof_id_type i = nodes.first;
    fa[i] = {};
    dfa[i] = {};
    for (const auto & neighbors : nodes.second)
    {
      const dof_id_type j = neighbors.first;
      fa[i][j] = 0.0;
      dfa[i][j] = {};
      // The derivatives are a bit complicated.
      // If i is upwind of j then fa[i][j] depends on all nodes connected to i.
      // But if i is downwind of j then fa[i][j] depends on all nodes connected to j.
      for (const auto & neighbor_to_i : _kij[i])
        dfa[i][j][neighbor_to_i.first] = 0.0;
      for (const auto & neighbor_to_j : _kij[j])
        dfa[i][j][neighbor_to_j.first] = 0.0;
    }
  }
  for (const auto & nodes : _kij)
  {
    const dof_id_type i = nodes.first;
    const Real u_i = _u_nodal->getNodalValue(_mesh.getMesh().node_ref(i));
    for (const auto & neighbors : nodes.second)
    {
      const dof_id_type j = neighbors.first;
      const Real u_j = _u_nodal->getNodalValue(_mesh.getMesh().node_ref(j));
      if (i == j)
        continue;
      if (lij[j][i] >= lij[i][j]) // node i is upwind of node j.
      {
        Real prefactor = 0.0;
        std::map<dof_id_type, Real>
            dprefactor_du; // dprefactor_dr[global_id] = d(prefactor)/du[global_id];
        for (const auto & dof_deriv : drP[i])
          dprefactor_du[dof_deriv.first] = 0.0;
        if (u_i >= u_j)
        {
          if (lij[j][i] <= rP[i] * dij[i][j])
            prefactor = lij[j][i];
          else
          {
            prefactor = rP[i] * dij[i][j];
            for (const auto & dof_deriv : drP[i])
              dprefactor_du[dof_deriv.first] = dof_deriv.second * dij[i][j];
          }
        }
        else
        {
          if (lij[j][i] <= rM[i] * dij[i][j])
            prefactor = lij[j][i];
          else
          {
            prefactor = rM[i] * dij[i][j];
            for (const auto & dof_deriv : drM[i])
              dprefactor_du[dof_deriv.first] = dof_deriv.second * dij[i][j];
          }
        }
        fa[i][j] = prefactor * (u_i - u_j);
        dfa[i][j][i] = prefactor;
        dfa[i][j][j] = -prefactor;
        for (const auto & dof_deriv : dprefactor_du)
          dfa[i][j][dof_deriv.first] += dof_deriv.second * (u_i - u_j);
      }
    }
  }
  for (const auto & nodes : _kij)
  {
    const dof_id_type i = nodes.first;
    for (const auto & neighbors : nodes.second)
    {
      const dof_id_type j = neighbors.first;
      if (i == j)
        continue;
      if (lij[j][i] < lij[i][j]) // node_i is downwind of node_j.
      {
        fa[i][j] = -fa[j][i];
        for (const auto & dof_deriv : dfa[j][i])
          dfa[i][j][dof_deriv.first] = -dof_deriv.second;
      }
    }
  }

  // zero _flux_out and _dflux_out_du
  for (const auto & nodes : _kij)
  {
    const dof_id_type i = nodes.first;
    _flux_out[i] = 0.0;
    // The derivatives are a bit complicated.
    // If i is upwind of a node "j" then _flux_out[i] depends on all nodes connected to i.
    // But if i is downwind of a node "j" then _flux_out depends on all nodes connected with node j.
    for (const auto & neighbors : nodes.second)
    {
      const dof_id_type j = neighbors.first;
      _dflux_out_du[i][j] = 0.0;
      for (const auto & neighbors_j : _kij[j])
        _dflux_out_du[i][neighbors_j.first] = 0.0;
    }
  }

  // Add everything together
  // See step 3 in Fig 2, noting Eqn (36)
  for (auto & nodes : _kij)
  {
    const dof_id_type i = nodes.first;
    for (const auto & neighbors : nodes.second)
    {
      const dof_id_type j = neighbors.first;
      const Real u_j = _u_nodal->getNodalValue(_mesh.getMesh().node_ref(j));

      // negative sign because residual = -Lu (KT equation (19))
      _flux_out[i] -= lij[i][j] * u_j + fa[i][j];

      _dflux_out_du[i][j] -= lij[i][j];
      for (const auto & dof_deriv : dfa[i][j])
        _dflux_out_du[i][dof_deriv.first] -= dof_deriv.second;
    }
  }
}
