/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "InitialCondition.h"
#include "FEProblem.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "libmesh/fe_interface.h"

template<>
InputParameters validParams<InitialCondition>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<BlockRestrictable>();
  params += validParams<BoundaryRestrictable>();

  params.addRequiredParam<VariableName>("variable", "The variable this initial condition is supposed to provide values for.");

  params.addPrivateParam<std::string>("built_by_action", "add_ic");
  return params;
}

InitialCondition::InitialCondition(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    Coupleable(parameters, getParam<SystemBase *>("_sys")->getVariable(parameters.get<THREAD_ID>("_tid"), parameters.get<VariableName>("variable")).isNodal()),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    BlockRestrictable(name, parameters),
    BoundaryRestrictable(name, parameters),
    DependencyResolverInterface(),
    Restartable(name, parameters, "InitialConditions"),
    ZeroInterface(parameters),
    _fe_problem(*getParam<FEProblem *>("_fe_problem")),
    _sys(*getParam<SystemBase *>("_sys")),
    _tid(getParam<THREAD_ID>("_tid")),
    _assembly(_fe_problem.assembly(_tid)),
    _t(_fe_problem.time()),
    _coord_sys(_assembly.coordSystem()),
    _var(_sys.getVariable(_tid, getParam<VariableName>("variable"))),

    _current_elem(_var.currentElem()),
    _qp(0)
{
  _supplied_vars.insert(getParam<VariableName>("variable"));

  std::map<std::string, std::vector<MooseVariable *> > coupled_vars = getCoupledVars();
  for (std::map<std::string, std::vector<MooseVariable *> >::iterator it = coupled_vars.begin(); it != coupled_vars.end(); ++it)
    for (std::vector<MooseVariable *>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
      _depend_vars.insert((*it2)->name());
}

InitialCondition::~InitialCondition()
{
}

const std::set<std::string> &
InitialCondition::getRequestedItems()
{
  return _depend_vars;
}

const std::set<std::string> &
InitialCondition::getSuppliedItems()
{
  return _supplied_vars;
}

void
InitialCondition::compute()
{
  // -- NOTE ----
  // The following code is a copy from libMesh project_vector.C plus it adds some features, so we can couple variable values
  // and we also do not call any callbacks, but we use our initial condition system directly.
  // ------------

  // The element matrix and RHS for projections.
  // Note that Ke is always real-valued, whereas Fe may be complex valued if complex number support is enabled
  DenseMatrix<Real> Ke;
  DenseVector<Number> Fe;
  // The new element coefficients
  DenseVector<Number> Ue;

  const FEType & fe_type = _var.feType();

  // The dimension of the current element
  const unsigned int dim = _current_elem->dim();
  // The element type
  const ElemType elem_type = _current_elem->type();
  // The number of nodes on the new element
  const unsigned int n_nodes = _current_elem->n_nodes();
  // The global DOF indices
  std::vector<unsigned int> dof_indices;
  // Side/edge DOF indices
  std::vector<unsigned int> side_dofs;

  // Get FE objects of the appropriate type
  // We cannot use the FE object in Assembly, since the following code is messing with the quadrature rules
  // for projections and would screw it up. However, if we implement projections from one mesh to another,
  // this code should use that implementation.
  AutoPtr<FEBase> fe (FEBase::build(dim, fe_type));

  // Prepare variables for projection
  AutoPtr<QBase> qrule     (fe_type.default_quadrature_rule(dim));
  AutoPtr<QBase> qedgerule (fe_type.default_quadrature_rule(1));
  AutoPtr<QBase> qsiderule (fe_type.default_quadrature_rule(dim-1));

  // The values of the shape functions at the quadrature points
  const std::vector<std::vector<Real> > & phi = fe->get_phi();

  // The gradients of the shape functions at the quadrature points on the child element.
  const std::vector<std::vector<RealGradient> > * dphi = NULL;

  const FEContinuity cont = fe->get_continuity();

  if (cont == C_ONE)
  {
    const std::vector<std::vector<RealGradient> > & ref_dphi = fe->get_dphi();
    dphi = &ref_dphi;
  }

  // The Jacobian * quadrature weight at the quadrature points
  const std::vector<Real> & JxW =  fe->get_JxW();
  // The XYZ locations of the quadrature points
  const std::vector<Point>& xyz_values = fe->get_xyz();

  // Update the DOF indices for this element based on the current mesh
  _var.prepareIC();
  dof_indices = _var.dofIndices();

  // The number of DOFs on the element
  const unsigned int n_dofs = dof_indices.size();
  if (n_dofs == 0)
    return;

  // Fixed vs. free DoFs on edge/face projections
  std::vector<char> dof_is_fixed(n_dofs, false); // bools
  std::vector<int> free_dof(n_dofs, 0);

  // Zero the interpolated values
  Ue.resize (n_dofs); Ue.zero();

  // In general, we need a series of
  // projections to ensure a unique and continuous
  // solution.  We start by interpolating nodes, then
  // hold those fixed and project edges, then
  // hold those fixed and project faces, then
  // hold those fixed and project interiors

  _fe_problem.sizeZeroes(n_nodes, _tid);

  // Interpolate node values first
  unsigned int current_dof = 0;
  for (unsigned int n = 0; n != n_nodes; ++n)
  {
    // FIXME: this should go through the DofMap,
    // not duplicate dof_indices code badly!
    const unsigned int nc = FEInterface::n_dofs_at_node (dim, fe_type, elem_type, n);
    if (!_current_elem->is_vertex(n))
    {
      current_dof += nc;
      continue;
    }
    if (cont == DISCONTINUOUS)
    {
      libmesh_assert(nc == 0);
    }
    // Assume that C_ZERO elements have a single nodal
    // value shape function
    else if (cont == C_ZERO)
    {
      libmesh_assert(nc == 1);
      _qp = n;
      Ue(current_dof) = value(_current_elem->point(n));
      dof_is_fixed[current_dof] = true;
      current_dof++;
    }
    // The hermite element vertex shape functions are weird
    else if (fe_type.family == HERMITE)
    {
      _qp = n;
      Ue(current_dof) = value(_current_elem->point(n));
      dof_is_fixed[current_dof] = true;
      current_dof++;
      Gradient grad = gradient(_current_elem->point(n));
      // x derivative
      Ue(current_dof) = grad(0);
      dof_is_fixed[current_dof] = true;
      current_dof++;
      if (dim > 1)
      {
        // We'll finite difference mixed derivatives
        Point nxminus = _current_elem->point(n),
              nxplus = _current_elem->point(n);
        nxminus(0) -= TOLERANCE;
        nxplus(0) += TOLERANCE;
        Gradient gxminus = gradient(nxminus);
        Gradient gxplus = gradient(nxplus);
        // y derivative
        Ue(current_dof) = grad(1);
        dof_is_fixed[current_dof] = true;
        current_dof++;
        // xy derivative
        Ue(current_dof) = (gxplus(1) - gxminus(1)) / 2. / TOLERANCE;
        dof_is_fixed[current_dof] = true;
        current_dof++;

        if (dim > 2)
        {
          // z derivative
          Ue(current_dof) = grad(2);
          dof_is_fixed[current_dof] = true;
          current_dof++;
          // xz derivative
          Ue(current_dof) = (gxplus(2) - gxminus(2)) / 2. / TOLERANCE;
          dof_is_fixed[current_dof] = true;
          current_dof++;
          // We need new points for yz
          Point nyminus = _current_elem->point(n),
                nyplus = _current_elem->point(n);
          nyminus(1) -= TOLERANCE;
          nyplus(1) += TOLERANCE;
          Gradient gyminus = gradient(nyminus);
          Gradient gyplus = gradient(nyplus);
          // xz derivative
          Ue(current_dof) = (gyplus(2) - gyminus(2)) / 2. / TOLERANCE;
          dof_is_fixed[current_dof] = true;
          current_dof++;
          // Getting a 2nd order xyz is more tedious
          Point nxmym = _current_elem->point(n),
                nxmyp = _current_elem->point(n),
                nxpym = _current_elem->point(n),
                nxpyp = _current_elem->point(n);
          nxmym(0) -= TOLERANCE;
          nxmym(1) -= TOLERANCE;
          nxmyp(0) -= TOLERANCE;
          nxmyp(1) += TOLERANCE;
          nxpym(0) += TOLERANCE;
          nxpym(1) -= TOLERANCE;
          nxpyp(0) += TOLERANCE;
          nxpyp(1) += TOLERANCE;
          Gradient gxmym = gradient(nxmym);
          Gradient gxmyp = gradient(nxmyp);
          Gradient gxpym = gradient(nxpym);
          Gradient gxpyp = gradient(nxpyp);
          Number gxzplus = (gxpyp(2) - gxmyp(2)) / 2. / TOLERANCE;
          Number gxzminus = (gxpym(2) - gxmym(2)) / 2. / TOLERANCE;
          // xyz derivative
          Ue(current_dof) = (gxzplus - gxzminus) / 2. / TOLERANCE;
          dof_is_fixed[current_dof] = true;
          current_dof++;
        }
      }
    }
    // Assume that other C_ONE elements have a single nodal
    // value shape function and nodal gradient component
    // shape functions
    else if (cont == C_ONE)
    {
      libmesh_assert(nc == 1 + dim);
      Ue(current_dof) = value(_current_elem->point(n));
      dof_is_fixed[current_dof] = true;
      current_dof++;
      Gradient grad = gradient(_current_elem->point(n));
      for (unsigned int i=0; i != dim; ++i)
      {
        Ue(current_dof) = grad(i);
        dof_is_fixed[current_dof] = true;
        current_dof++;
      }
    }
    else
      libmesh_error();
  } // loop over nodes

  // In 3D, project any edge values next
  if (dim > 2 && cont != DISCONTINUOUS)
    for (unsigned int e=0; e != _current_elem->n_edges(); ++e)
    {
      FEInterface::dofs_on_edge(_current_elem, dim, fe_type, e, side_dofs);

      // Some edge dofs are on nodes and already
      // fixed, others are free to calculate
      unsigned int free_dofs = 0;
      for (unsigned int i=0; i != side_dofs.size(); ++i)
        if (!dof_is_fixed[side_dofs[i]])
          free_dof[free_dofs++] = i;

      // There may be nothing to project
      if (!free_dofs)
        continue;

      Ke.resize (free_dofs, free_dofs); Ke.zero();
      Fe.resize (free_dofs); Fe.zero();
      // The new edge coefficients
      DenseVector<Number> Uedge(free_dofs);

      // Initialize FE data on the edge
      fe->attach_quadrature_rule (qedgerule.get());
      fe->edge_reinit (_current_elem, e);
      const unsigned int n_qp = qedgerule->n_points();
      _fe_problem.sizeZeroes(n_qp, _tid);

      // Loop over the quadrature points
      for (unsigned int qp = 0; qp < n_qp; qp++)
      {
        // solution at the quadrature point
        Number fineval = value(xyz_values[qp]);
        // solution grad at the quadrature point
        Gradient finegrad;
        if (cont == C_ONE)
          finegrad = gradient(xyz_values[qp]);

        // Form edge projection matrix
        for (unsigned int sidei = 0, freei = 0; sidei != side_dofs.size(); ++sidei)
        {
          unsigned int i = side_dofs[sidei];
          // fixed DoFs aren't test functions
          if (dof_is_fixed[i])
            continue;
          for (unsigned int sidej = 0, freej = 0; sidej != side_dofs.size(); ++sidej)
          {
            unsigned int j = side_dofs[sidej];
            if (dof_is_fixed[j])
              Fe(freei) -= phi[i][qp] * phi[j][qp] * JxW[qp] * Ue(j);
            else
              Ke(freei,freej) += phi[i][qp] * phi[j][qp] * JxW[qp];
            if (cont == C_ONE)
            {
              if (dof_is_fixed[j])
                Fe(freei) -= ((*dphi)[i][qp] * (*dphi)[j][qp]) * JxW[qp] * Ue(j);
              else
                Ke(freei,freej) += ((*dphi)[i][qp] * (*dphi)[j][qp]) * JxW[qp];
            }
            if (!dof_is_fixed[j])
              freej++;
          }
          Fe(freei) += phi[i][qp] * fineval * JxW[qp];
          if (cont == C_ONE)
            Fe(freei) += (finegrad * (*dphi)[i][qp]) * JxW[qp];
          freei++;
        }
      }

      Ke.cholesky_solve(Fe, Uedge);

      // Transfer new edge solutions to element
      for (unsigned int i=0; i != free_dofs; ++i)
      {
        Number &ui = Ue(side_dofs[free_dof[i]]);
        libmesh_assert(std::abs(ui) < TOLERANCE || std::abs(ui - Uedge(i)) < TOLERANCE);
        ui = Uedge(i);
        dof_is_fixed[side_dofs[free_dof[i]]] = true;
      }
    }

  // Project any side values (edges in 2D, faces in 3D)
  if (dim > 1 && cont != DISCONTINUOUS)
    for (unsigned int s=0; s != _current_elem->n_sides(); ++s)
    {
      FEInterface::dofs_on_side(_current_elem, dim, fe_type, s, side_dofs);

      // Some side dofs are on nodes/edges and already
      // fixed, others are free to calculate
      unsigned int free_dofs = 0;
      for (unsigned int i=0; i != side_dofs.size(); ++i)
        if (!dof_is_fixed[side_dofs[i]])
          free_dof[free_dofs++] = i;

      // There may be nothing to project
      if (!free_dofs)
        continue;

      Ke.resize (free_dofs, free_dofs); Ke.zero();
      Fe.resize (free_dofs); Fe.zero();
      // The new side coefficients
      DenseVector<Number> Uside(free_dofs);

      // Initialize FE data on the side
      fe->attach_quadrature_rule (qsiderule.get());
      fe->reinit (_current_elem, s);
      const unsigned int n_qp = qsiderule->n_points();
      _fe_problem.sizeZeroes(n_qp, _tid);

      // Loop over the quadrature points
      for (unsigned int qp = 0; qp < n_qp; qp++)
      {
        // solution at the quadrature point
        Number fineval = value(xyz_values[qp]);
        // solution grad at the quadrature point
        Gradient finegrad;
        if (cont == C_ONE)
          finegrad = gradient(xyz_values[qp]);

        // Form side projection matrix
        for (unsigned int sidei = 0, freei = 0; sidei != side_dofs.size(); ++sidei)
        {
          unsigned int i = side_dofs[sidei];
          // fixed DoFs aren't test functions
          if (dof_is_fixed[i])
            continue;
          for (unsigned int sidej = 0, freej = 0; sidej != side_dofs.size(); ++sidej)
          {
            unsigned int j = side_dofs[sidej];
            if (dof_is_fixed[j])
              Fe(freei) -= phi[i][qp] * phi[j][qp] * JxW[qp] * Ue(j);
            else
              Ke(freei,freej) += phi[i][qp] * phi[j][qp] * JxW[qp];
            if (cont == C_ONE)
            {
              if (dof_is_fixed[j])
                Fe(freei) -= ((*dphi)[i][qp] * (*dphi)[j][qp]) * JxW[qp] * Ue(j);
              else
                Ke(freei,freej) += ((*dphi)[i][qp] * (*dphi)[j][qp]) * JxW[qp];
            }
            if (!dof_is_fixed[j])
              freej++;
          }
          Fe(freei) += (fineval * phi[i][qp]) * JxW[qp];
          if (cont == C_ONE)
            Fe(freei) += (finegrad * (*dphi)[i][qp]) * JxW[qp];
          freei++;
        }
      }

      Ke.cholesky_solve(Fe, Uside);

      // Transfer new side solutions to element
      for (unsigned int i=0; i != free_dofs; ++i)
      {
        Number &ui = Ue(side_dofs[free_dof[i]]);
        libmesh_assert(std::abs(ui) < TOLERANCE || std::abs(ui - Uside(i)) < TOLERANCE);
        ui = Uside(i);
        dof_is_fixed[side_dofs[free_dof[i]]] = true;
      }
    }

  // Project the interior values, finally

  // Some interior dofs are on nodes/edges/sides and
  // already fixed, others are free to calculate
  unsigned int free_dofs = 0;
  for (unsigned int i=0; i != n_dofs; ++i)
    if (!dof_is_fixed[i])
      free_dof[free_dofs++] = i;

  // There may be nothing to project
  if (free_dofs)
  {
    Ke.resize (free_dofs, free_dofs); Ke.zero();
    Fe.resize (free_dofs); Fe.zero();
    // The new interior coefficients
    DenseVector<Number> Uint(free_dofs);

    // Initialize FE data
    fe->attach_quadrature_rule (qrule.get());
    fe->reinit (_current_elem);
    const unsigned int n_qp = qrule->n_points();
    _fe_problem.sizeZeroes(n_qp, _tid);

    // Loop over the quadrature points
    for (unsigned int qp=0; qp<n_qp; qp++)
    {
      // solution at the quadrature point
      Number fineval = value(xyz_values[qp]);
      // solution grad at the quadrature point
      Gradient finegrad;
      if (cont == C_ONE)
        finegrad = gradient(xyz_values[qp]);

      // Form interior projection matrix
      for (unsigned int i=0, freei=0; i != n_dofs; ++i)
      {
        // fixed DoFs aren't test functions
        if (dof_is_fixed[i])
          continue;
        for (unsigned int j=0, freej=0; j != n_dofs; ++j)
        {
          if (dof_is_fixed[j])
            Fe(freei) -= phi[i][qp] * phi[j][qp] * JxW[qp] * Ue(j);
          else
            Ke(freei,freej) += phi[i][qp] * phi[j][qp] * JxW[qp];
          if (cont == C_ONE)
          {
            if (dof_is_fixed[j])
              Fe(freei) -= ((*dphi)[i][qp] * (*dphi)[j][qp]) * JxW[qp] * Ue(j);
            else
              Ke(freei,freej) += ((*dphi)[i][qp] * (*dphi)[j][qp]) * JxW[qp];
          }
          if (!dof_is_fixed[j])
            freej++;
        }
        Fe(freei) += phi[i][qp] * fineval * JxW[qp];
        if (cont == C_ONE)
          Fe(freei) += (finegrad * (*dphi)[i][qp]) * JxW[qp];
        freei++;
      }
    }
    Ke.cholesky_solve(Fe, Uint);

    // Transfer new interior solutions to element
    for (unsigned int i=0; i != free_dofs; ++i)
    {
      Number &ui = Ue(free_dof[i]);
      libmesh_assert(std::abs(ui) < TOLERANCE || std::abs(ui - Uint(i)) < TOLERANCE);
      ui = Uint(i);
      dof_is_fixed[free_dof[i]] = true;
    }
  } // if there are free interior dofs

  // Make sure every DoF got reached!
  for (unsigned int i=0; i != n_dofs; ++i)
    libmesh_assert(dof_is_fixed[i]);

  NumericVector<Number> & solution = _var.sys().solution();

  const unsigned int
    first = solution.first_local_index(),
    last  = solution.last_local_index();

  // Lock the new_vector since it is shared among threads.
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

    for (unsigned int i = 0; i < n_dofs; i++)
      // We may be projecting a new zero value onto
      // an old nonzero approximation - RHS
      // if (Ue(i) != 0.)
      if ((dof_indices[i] >= first) && (dof_indices[i] < last))
      {
        solution.set(dof_indices[i], Ue(i));
        if (cont == C_ZERO)
          _var.setNodalValue(Ue(i), i);
      }
  }
}
