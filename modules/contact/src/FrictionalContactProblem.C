/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// MOOSE includes
#include "FrictionalContactProblem.h"

#include "AuxiliarySystem.h"
#include "DisplacedProblem.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "NearestNodeLocator.h"
#include "NonlinearSystem.h"
#include "PenetrationLocator.h"

#include <limits>

template <>
InputParameters
validParams<FrictionalContactProblem>()
{
  InputParameters params = validParams<ReferenceResidualProblem>();
  params.addRequiredParam<std::vector<int>>(
      "master", "IDs of the master surfaces for which the slip should be calculated");
  params.addRequiredParam<std::vector<int>>(
      "slave", "IDs of the slave surfaces for which the slip should be calculated");
  params.addRequiredParam<std::vector<Real>>(
      "friction_coefficient", "Coefficient of friction for sliding contact for each interaction");
  params.addRequiredParam<std::vector<Real>>(
      "slip_factor", "Fraction of calculated slip to be applied for each interaction");
  params.addRequiredParam<std::vector<Real>>("slip_too_far_factor",
                                             "Fraction of calculated slip to be applied for each "
                                             "interaction in the slipped-too-far state");
  params.addRequiredParam<NonlinearVariableName>("disp_x",
                                                 "Variable containing the x displacement");
  params.addRequiredParam<NonlinearVariableName>("disp_y",
                                                 "Variable containing the y displacement");
  params.addParam<NonlinearVariableName>("disp_z", "Variable containing the z displacement");
  params.addRequiredParam<AuxVariableName>("residual_x",
                                           "Auxiliary variable containing the saved x residual");
  params.addRequiredParam<AuxVariableName>("residual_y",
                                           "Auxiliary variable containing the saved y residual");
  params.addParam<AuxVariableName>("residual_z",
                                   "Auxiliary variable containing the saved z residual");
  params.addRequiredParam<AuxVariableName>(
      "diag_stiff_x", "Auxiliary variable containing the saved x diagonal stiffness");
  params.addRequiredParam<AuxVariableName>(
      "diag_stiff_y", "Auxiliary variable containing the saved y diagonal stiffness");
  params.addParam<AuxVariableName>("diag_stiff_z",
                                   "Auxiliary variable containing the saved z diagonal stiffness");
  params.addRequiredParam<AuxVariableName>("inc_slip_x",
                                           "Auxiliary variable to store the x incremental slip");
  params.addRequiredParam<AuxVariableName>("inc_slip_y",
                                           "Auxiliary variable to store the y incremental slip");
  params.addParam<AuxVariableName>("inc_slip_z",
                                   "Auxiliary variable to store the z incremental slip");
  params.addParam<int>("minimum_slip_iterations", 1, "Minimum number of slip iterations per step");
  params.addParam<int>(
      "maximum_slip_iterations", 100, "Maximum number of slip iterations per step");
  params.addParam<int>(
      "slip_updates_per_iteration", 1, "Number of slip updates per contact iteration");
  params.addParam<Real>("target_contact_residual",
                        "Frictional contact residual convergence criterion");
  params.addParam<Real>("target_relative_contact_residual",
                        "Frictional contact relative residual convergence criterion");
  params.addParam<Real>("contact_slip_tolerance_factor",
                        10.0,
                        "Multiplier on convergence criteria to determine when to start slipping");
  params.addParam<std::vector<std::string>>(
      "contact_reference_residual_variables",
      "Set of variables that provide reference residuals for relative contact convergence check");
  return params;
}

FrictionalContactProblem::SlipData::SlipData(const Node * node, unsigned int dof, Real slip)
  : _node(node), _dof(dof), _slip(slip)
{
}

FrictionalContactProblem::SlipData::SlipData(const SlipData & sd)
  : _node(sd._node), _dof(sd._dof), _slip(sd._slip)
{
}

FrictionalContactProblem::SlipData::~SlipData() {}

FrictionalContactProblem::FrictionalContactProblem(const InputParameters & params)
  : ReferenceResidualProblem(params),
    _refResidContact(0.0),
    _slip_residual(0.0),
    _do_slip_update(false),
    _num_slip_iterations(0),
    _target_contact_residual(0.0),
    _target_relative_contact_residual(0.0),
    _num_nl_its_since_contact_update(0),
    _num_contact_nodes(0),
    _num_slipping(0),
    _num_slipped_too_far(0),
    _inc_slip_norm(0.0),
    _it_slip_norm(0.0)
{
  std::vector<int> master = params.get<std::vector<int>>("master");
  std::vector<int> slave = params.get<std::vector<int>>("slave");
  std::vector<Real> friction_coefficient = params.get<std::vector<Real>>("friction_coefficient");
  std::vector<Real> slip_factor = params.get<std::vector<Real>>("slip_factor");
  std::vector<Real> slip_too_far_factor = params.get<std::vector<Real>>("slip_too_far_factor");

  unsigned int dim = getNonlinearSystemBase().subproblem().mesh().dimension();

  _disp_x = params.get<NonlinearVariableName>("disp_x");
  _residual_x = params.get<AuxVariableName>("residual_x");
  _diag_stiff_x = params.get<AuxVariableName>("diag_stiff_x");
  _inc_slip_x = params.get<AuxVariableName>("inc_slip_x");

  _disp_y = params.get<NonlinearVariableName>("disp_y");
  _residual_y = params.get<AuxVariableName>("residual_y");
  _diag_stiff_y = params.get<AuxVariableName>("diag_stiff_y");
  _inc_slip_y = params.get<AuxVariableName>("inc_slip_y");

  if (dim == 3)
  {
    if (!params.isParamValid("disp_z"))
      mooseError("Missing disp_z in FrictionalContactProblem");
    if (!params.isParamValid("residual_z"))
      mooseError("Missing residual_z in FrictionalContactProblem");
    if (!params.isParamValid("diag_stiff_z"))
      mooseError("Missing diag_stiff_z in FrictionalContactProblem");
    if (!params.isParamValid("inc_slip_z"))
      mooseError("Missing inc_slip_z in FrictionalContactProblem");
    _disp_z = params.get<NonlinearVariableName>("disp_z");
    _residual_z = params.get<AuxVariableName>("residual_z");
    _diag_stiff_z = params.get<AuxVariableName>("diag_stiff_z");
    _inc_slip_z = params.get<AuxVariableName>("inc_slip_z");
  }

  unsigned int num_interactions = master.size();
  if (num_interactions != slave.size())
    mooseError(
        "Sizes of master surface and slave surface lists must match in FrictionalContactProblem");
  if (num_interactions != friction_coefficient.size())
    mooseError(
        "Must have friction coefficient defined for every interaction in FrictionalContactProblem");
  if (num_interactions != slip_factor.size())
    mooseError("Must have slip factor defined for every interaction in FrictionalContactProblem");
  if (num_interactions != slip_too_far_factor.size())
    mooseError(
        "Must have slip too far factor defined for every interaction in FrictionalContactProblem");

  for (unsigned int i = 0; i < master.size(); ++i)
  {
    std::pair<int, int> ms_pair(master[i], slave[i]);
    InteractionParams ip;
    ip._friction_coefficient = friction_coefficient[i];
    ip._slip_factor = slip_factor[i];
    ip._slip_too_far_factor = slip_too_far_factor[i];

    _interaction_params[ms_pair] = ip;
  }

  _min_slip_iters = params.get<int>("minimum_slip_iterations");
  _max_slip_iters = params.get<int>("maximum_slip_iterations");
  _slip_updates_per_iter = params.get<int>("slip_updates_per_iteration");

  bool have_target = false;
  bool have_target_relative = false;
  if (params.isParamValid("target_contact_residual"))
  {
    _target_contact_residual = params.get<Real>("target_contact_residual");
    have_target = true;
  }
  if (params.isParamValid("target_relative_contact_residual"))
  {
    _target_relative_contact_residual = params.get<Real>("target_relative_contact_residual");
    have_target_relative = true;
  }
  if (!(have_target || have_target_relative))
    mooseError("Must specify either target_contact_residual or target_relative_contact_residual");

  _contact_slip_tol_factor = params.get<Real>("contact_slip_tolerance_factor");

  if (params.isParamValid("contact_reference_residual_variables"))
    _contactRefResidVarNames =
        params.get<std::vector<std::string>>("contact_reference_residual_variables");
}

void
FrictionalContactProblem::initialSetup()
{
  ReferenceResidualProblem::initialSetup();

  _contactRefResidVarIndices.clear();
  for (unsigned int i = 0; i < _contactRefResidVarNames.size(); ++i)
  {
    bool foundMatch = false;
    for (unsigned int j = 0; j < _refResidVarNames.size(); ++j)
    {
      if (_contactRefResidVarNames[i] == _refResidVarNames[j])
      {
        _contactRefResidVarIndices.push_back(j);
        foundMatch = true;
        break;
      }
    }
    if (!foundMatch)
      mooseError("Could not find variable '",
                 _contactRefResidVarNames[i],
                 "' in reference_residual_variables");
  }

  //  if (_contactRefResidVarIndices.size()>0)
  //  {
  //    Moose::out << "Contact reference convergence variables:" << std::endl;
  //    for (unsigned int i=0; i<_contactRefResidVarIndices.size(); ++i)
  //    {
  //      _console << _contactRefResidVarNames[i] << std::endl;;
  //    }
  //  }
}

void
FrictionalContactProblem::timestepSetup()
{
  _do_slip_update = false;
  _num_slip_iterations = 0;
  _num_nl_its_since_contact_update = 0;
  _refResidContact = 0.0;
  ReferenceResidualProblem::timestepSetup();
}

void
FrictionalContactProblem::updateContactReferenceResidual()
{
  if (_contactRefResidVarIndices.size() > 0)
  {
    _refResidContact = 0.0;
    for (unsigned int i = 0; i < _contactRefResidVarIndices.size(); ++i)
      _refResidContact += _refResid[i] * _refResid[i];

    _refResidContact = std::sqrt(_refResidContact);
  }
  else if (_refResid.size() > 0)
  {
    _refResidContact = 0.0;
    for (unsigned int i = 0; i < _refResid.size(); ++i)
      _refResidContact += _refResid[i] * _refResid[i];

    _refResidContact = std::sqrt(_refResidContact);
  }
  _console << "Contact reference convergence residual: " << _refResidContact << std::endl;
  ;
}

bool
FrictionalContactProblem::shouldUpdateSolution()
{
  return true;
}

bool
FrictionalContactProblem::updateSolution(NumericVector<Number> & vec_solution,
                                         NumericVector<Number> & ghosted_solution)
{
  bool solution_modified = false;

  solution_modified |= enforceRateConstraint(vec_solution, ghosted_solution);

  unsigned int nfc = numLocalFrictionalConstraints();
  std::vector<SlipData> iterative_slip;
  unsigned int dim = getNonlinearSystemBase().subproblem().mesh().dimension();
  iterative_slip.reserve(nfc * dim);

  if (_do_slip_update)
  {
    updateReferenceResidual();
    updateContactReferenceResidual();
    _console << "Slip Update: " << _num_slip_iterations << std::endl;
    _console << "Iter  #Cont     #Slip     #TooFar   Slip resid  Inc Slip    It Slip" << std::endl;

    for (int i = 0; i < _slip_updates_per_iter; i++)
    {
      _console << std::left << std::setw(6) << i + 1;

      bool updated_this_iter = calculateSlip(ghosted_solution, &iterative_slip);

      _console << std::setw(10) << _num_contact_nodes << std::setw(10) << _num_slipping
               << std::setw(10) << _num_slipped_too_far << std::setprecision(4) << std::setw(12)
               << _slip_residual << std::setw(12) << _inc_slip_norm << std::setw(12)
               << _it_slip_norm;

      if (updated_this_iter)
      {
        if (_slip_residual < _target_contact_residual ||
            _slip_residual < _target_relative_contact_residual * _refResidContact)
        {
          _console << "     Converged: Slip resid < tolerance, not applying this slip update"
                   << std::endl;
          break;
        }
        else
        {
          _console << std::endl;
          applySlip(vec_solution, ghosted_solution, iterative_slip);
        }
      }
      else
      {
        _console << "     Converged: No slipping nodes" << std::endl;
        break;
      }

      solution_modified |= updated_this_iter;
    }
    _num_slip_iterations++;
    _do_slip_update = false;
    _num_nl_its_since_contact_update = 0;
  }

  return solution_modified;
}

void
FrictionalContactProblem::predictorCleanup(NumericVector<Number> & ghosted_solution)
{
  updateContactPoints(ghosted_solution, true);
}

bool
FrictionalContactProblem::enforceRateConstraint(NumericVector<Number> & vec_solution,
                                                NumericVector<Number> & ghosted_solution)
{
  NonlinearSystemBase & nonlinear_sys = getNonlinearSystemBase();
  unsigned int dim = nonlinear_sys.subproblem().mesh().dimension();

  _displaced_problem->updateMesh(ghosted_solution, *_aux->currentSolution());

  MooseVariable * disp_x_var = &getVariable(0, _disp_x);
  MooseVariable * disp_y_var = &getVariable(0, _disp_y);
  MooseVariable * disp_z_var = nullptr;
  if (dim == 3)
    disp_z_var = &getVariable(0, _disp_z);

  bool updatedSolution = false;

  if (getDisplacedProblem() && _interaction_params.size() > 0)
  {
    GeometricSearchData & displaced_geom_search_data = getDisplacedProblem()->geomSearchData();
    std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators =
        &displaced_geom_search_data._penetration_locators;

    for (PLIterator plit = penetration_locators->begin(); plit != penetration_locators->end();
         ++plit)
    {
      PenetrationLocator & pen_loc = *plit->second;

      bool frictional_contact_this_interaction = false;

      std::map<std::pair<int, int>, InteractionParams>::iterator ipit;
      std::pair<int, int> ms_pair(pen_loc._master_boundary, pen_loc._slave_boundary);
      ipit = _interaction_params.find(ms_pair);

      if (ipit != _interaction_params.end())
        frictional_contact_this_interaction = true;

      if (frictional_contact_this_interaction)
      {
        std::vector<dof_id_type> & slave_nodes = pen_loc._nearest_node._slave_nodes;

        for (unsigned int i = 0; i < slave_nodes.size(); i++)
        {
          dof_id_type slave_node_num = slave_nodes[i];

          if (pen_loc._penetration_info[slave_node_num])
          {
            PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];

            if (info.isCaptured())
            {
              // _console << "Slave node: " << slave_node_num << std::endl;
              const Node * node = info._node;

              VectorValue<dof_id_type> solution_dofs(
                  node->dof_number(nonlinear_sys.number(), disp_x_var->number(), 0),
                  node->dof_number(nonlinear_sys.number(), disp_y_var->number(), 0),
                  (disp_z_var ? node->dof_number(nonlinear_sys.number(), disp_z_var->number(), 0)
                              : 0));

              // Get old parametric coords from info
              // get old element from info
              // Apply current configuration to that element and find xyz coords of that point
              // find xyz coords of current point from original coords and displacement vector
              // calculate difference between those two point coordinates to get correction

              // std::vector<Point>points(1);
              // points[0] = info._starting_closest_point_ref;
              // Elem *side =
              // (info._starting_elem->build_side(info._starting_side_num,false)).release();
              // FEBase &fe = *(pen_loc._fe[0]);
              // fe.reinit(side, &points);
              // RealVectorValue correction = starting_point[0] - current_coords;
              //
              // RealVectorValue current_coords = *node;
              // RealVectorValue correction = info._closest_point - current_coords;

              const Node & undisp_node = _mesh.nodeRef(node->id());
              RealVectorValue solution = info._closest_point - undisp_node;

              for (unsigned int i = 0; i < dim; ++i)
              {
                //                  vec_solution.add(solution_dofs(i), correction(i));
                vec_solution.set(solution_dofs(i), solution(i));
              }
              info._distance = 0.0;
            }
          }
        }
      }

      updatedSolution = true;
    }

    vec_solution.close();

    _communicator.max(updatedSolution);

    if (updatedSolution)
    {
      ghosted_solution = vec_solution;
      ghosted_solution.close();
    }
  }

  return updatedSolution;
}

bool
FrictionalContactProblem::calculateSlip(const NumericVector<Number> & ghosted_solution,
                                        std::vector<SlipData> * iterative_slip)
{
  NonlinearSystemBase & nonlinear_sys = getNonlinearSystemBase();
  unsigned int dim = nonlinear_sys.subproblem().mesh().dimension();

  MooseVariable * residual_x_var = &getVariable(0, _residual_x);
  MooseVariable * residual_y_var = &getVariable(0, _residual_y);
  MooseVariable * residual_z_var = nullptr;
  MooseVariable * diag_stiff_x_var = &getVariable(0, _diag_stiff_x);
  MooseVariable * diag_stiff_y_var = &getVariable(0, _diag_stiff_y);
  MooseVariable * diag_stiff_z_var = nullptr;
  MooseVariable * inc_slip_x_var = &getVariable(0, _inc_slip_x);
  MooseVariable * inc_slip_y_var = &getVariable(0, _inc_slip_y);
  MooseVariable * inc_slip_z_var = nullptr;
  if (dim == 3)
  {
    residual_z_var = &getVariable(0, _residual_z);
    diag_stiff_z_var = &getVariable(0, _diag_stiff_z);
    inc_slip_z_var = &getVariable(0, _inc_slip_z);
  }

  bool updatedSolution = false;
  _slip_residual = 0.0;
  _it_slip_norm = 0.0;
  _inc_slip_norm = 0.0;

  if (iterative_slip)
    iterative_slip->clear();

  if (getDisplacedProblem() && _interaction_params.size() > 0)
  {
    computeResidual(ghosted_solution, getNonlinearSystemBase().RHS());

    _num_contact_nodes = 0;
    _num_slipping = 0;
    _num_slipped_too_far = 0;

    GeometricSearchData & displaced_geom_search_data = getDisplacedProblem()->geomSearchData();
    std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators =
        &displaced_geom_search_data._penetration_locators;

    AuxiliarySystem & aux_sys = getAuxiliarySystem();
    const NumericVector<Number> & aux_solution = *aux_sys.currentSolution();

    for (PLIterator plit = penetration_locators->begin(); plit != penetration_locators->end();
         ++plit)
    {
      PenetrationLocator & pen_loc = *plit->second;

      bool frictional_contact_this_interaction = false;

      std::map<std::pair<int, int>, InteractionParams>::iterator ipit;
      std::pair<int, int> ms_pair(pen_loc._master_boundary, pen_loc._slave_boundary);
      ipit = _interaction_params.find(ms_pair);
      if (ipit != _interaction_params.end())
        frictional_contact_this_interaction = true;

      if (frictional_contact_this_interaction)
      {

        InteractionParams & interaction_params = ipit->second;
        Real slip_factor = interaction_params._slip_factor;
        Real slip_too_far_factor = interaction_params._slip_too_far_factor;
        Real friction_coefficient = interaction_params._friction_coefficient;

        std::vector<dof_id_type> & slave_nodes = pen_loc._nearest_node._slave_nodes;

        for (unsigned int i = 0; i < slave_nodes.size(); i++)
        {
          dof_id_type slave_node_num = slave_nodes[i];

          if (pen_loc._penetration_info[slave_node_num])
          {
            PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];
            const Node * node = info._node;

            if (node->processor_id() == processor_id())
            {

              if (info.isCaptured())
              {
                _num_contact_nodes++;

                VectorValue<dof_id_type> residual_dofs(
                    node->dof_number(aux_sys.number(), residual_x_var->number(), 0),
                    node->dof_number(aux_sys.number(), residual_y_var->number(), 0),
                    (residual_z_var
                         ? node->dof_number(aux_sys.number(), residual_z_var->number(), 0)
                         : 0));

                VectorValue<dof_id_type> diag_stiff_dofs(
                    node->dof_number(aux_sys.number(), diag_stiff_x_var->number(), 0),
                    node->dof_number(aux_sys.number(), diag_stiff_y_var->number(), 0),
                    (diag_stiff_z_var
                         ? node->dof_number(aux_sys.number(), diag_stiff_z_var->number(), 0)
                         : 0));

                VectorValue<dof_id_type> inc_slip_dofs(
                    node->dof_number(aux_sys.number(), inc_slip_x_var->number(), 0),
                    node->dof_number(aux_sys.number(), inc_slip_y_var->number(), 0),
                    (inc_slip_z_var
                         ? node->dof_number(aux_sys.number(), inc_slip_z_var->number(), 0)
                         : 0));

                RealVectorValue res_vec;
                RealVectorValue stiff_vec;
                RealVectorValue slip_inc_vec;

                for (unsigned int i = 0; i < dim; ++i)
                {
                  res_vec(i) = aux_solution(residual_dofs(i));
                  stiff_vec(i) = aux_solution(diag_stiff_dofs(i));
                  slip_inc_vec(i) = aux_solution(inc_slip_dofs(i));
                }

                RealVectorValue slip_iterative(0.0, 0.0, 0.0);
                Real interaction_slip_residual = 0.0;
                //  _console << "inc  slip: " << slip_inc_vec << std::endl;
                //  _console << "info slip: " << info._incremental_slip << std::endl;
                //  ContactState state = calculateInteractionSlip(slip_iterative,
                //  interaction_slip_residual, info._normal, res_vec, info._incremental_slip,
                //  stiff_vec, friction_coefficient, slip_factor, slip_too_far_factor, dim);
                ContactState state = calculateInteractionSlip(slip_iterative,
                                                              interaction_slip_residual,
                                                              info._normal,
                                                              res_vec,
                                                              slip_inc_vec,
                                                              stiff_vec,
                                                              friction_coefficient,
                                                              slip_factor,
                                                              slip_too_far_factor,
                                                              dim);
                // _console << "iter slip: " << slip_iterative << std::endl;
                _slip_residual += interaction_slip_residual * interaction_slip_residual;

                if (state == SLIPPING || state == SLIPPED_TOO_FAR)
                {
                  _num_slipping++;
                  if (state == SLIPPED_TOO_FAR)
                    _num_slipped_too_far++;
                  for (unsigned int i = 0; i < dim; ++i)
                  {
                    SlipData sd(node, i, slip_iterative(i));
                    if (iterative_slip)
                      iterative_slip->push_back(sd);
                    _it_slip_norm += slip_iterative(i) * slip_iterative(i);
                    _inc_slip_norm += (slip_inc_vec(i) + slip_iterative(i)) *
                                      (slip_inc_vec(i) + slip_iterative(i));
                  }
                }
              }
            }
          }
        }
      }
    }

    _communicator.sum(_num_contact_nodes);
    _communicator.sum(_num_slipping);
    _communicator.sum(_num_slipped_too_far);
    _communicator.sum(_slip_residual);
    _slip_residual = std::sqrt(_slip_residual);
    _communicator.sum(_it_slip_norm);
    _it_slip_norm = std::sqrt(_it_slip_norm);
    _communicator.sum(_inc_slip_norm);
    _inc_slip_norm = std::sqrt(_inc_slip_norm);
    if (_num_slipping > 0)
      updatedSolution = true;
  }

  return updatedSolution;
}

FrictionalContactProblem::ContactState
FrictionalContactProblem::calculateInteractionSlip(RealVectorValue & slip,
                                                   Real & slip_residual,
                                                   const RealVectorValue & normal,
                                                   const RealVectorValue & residual,
                                                   const RealVectorValue & incremental_slip,
                                                   const RealVectorValue & stiffness,
                                                   const Real friction_coefficient,
                                                   const Real slip_factor,
                                                   const Real slip_too_far_factor,
                                                   const int dim)
{

  ContactState state = STICKING;

  RealVectorValue normal_residual = normal * (normal * residual);
  Real normal_force = normal_residual.norm();

  // _console << "normal=" << info._normal << std::endl;
  // _console << "normal_force=" << normal_force << std::endl;
  // _console << "residual=" << residual << std::endl;
  // _console << "stiffness=" << stiff_vec << std::endl;

  RealVectorValue tangential_force =
      normal_residual - residual; // swap sign to make the code more manageable
  Real tangential_force_magnitude = tangential_force.norm();

  Real capacity = normal_force * friction_coefficient;
  if (capacity < 0.0)
    capacity = 0.0;

  Real slip_inc = incremental_slip.norm();

  slip(0) = 0.0;
  slip(1) = 0.0;
  slip(2) = 0.0;

  if (slip_inc > 0)
  {
    state = SLIPPING;
    Real slip_dot_tang_force = incremental_slip * tangential_force / slip_inc;
    if (slip_dot_tang_force < capacity)
    {
      state = SLIPPED_TOO_FAR;
      // _console << "STF slip_dot_force: " << slip_dot_tang_force << " capacity: " << capacity <<
      // std::endl;
    }
  }

  Real excess_force = tangential_force_magnitude - capacity;
  if (excess_force < 0.0)
    excess_force = 0;

  if (state == SLIPPED_TOO_FAR)
  {
    RealVectorValue slip_inc_direction = incremental_slip / slip_inc;
    Real tangential_force_in_slip_dir = slip_inc_direction * tangential_force;
    slip_residual = capacity - tangential_force_in_slip_dir;

    RealVectorValue force_from_unit_slip(0.0, 0.0, 0.0);
    for (int i = 0; i < dim; ++i)
      force_from_unit_slip(i) = stiffness(i) * slip_inc_direction(i);

    Real stiffness_slipdir =
        force_from_unit_slip *
        slip_inc_direction; // k=f resolved to slip dir because f=kd and d is a unit vector

    Real slip_distance =
        slip_too_far_factor * (capacity - tangential_force_in_slip_dir) / stiffness_slipdir;
    //    _console << "STF dist: " << slip_distance << " inc: " << slip_inc << std::endl;
    //    _console << "STF  cap: " << capacity << " tfs: " << tangential_force_in_slip_dir <<
    //    std::endl;
    if (slip_distance < slip_inc)
    {
      //      _console << "STF" << std::endl;
      slip = slip_inc_direction * -slip_distance;
    }
    else
    {
      //      _console << "STF max" << std::endl;
      slip = -incremental_slip;
    }
  }
  else if (excess_force > 0)
  {
    state = SLIPPING;
    Real tangential_force_magnitude = tangential_force.norm();
    slip_residual = excess_force;

    RealVectorValue tangential_direction = tangential_force / tangential_force_magnitude;
    RealVectorValue excess_force_vector = tangential_direction * excess_force;

    for (int i = 0; i < dim; ++i)
      slip(i) = slip_factor * excess_force_vector(i) / stiffness(i);

    // zero out component of slip in normal direction
    RealVectorValue slip_normal_dir = normal * (normal * slip);
    slip = slip - slip_normal_dir;
  }
  return state;
}

void
FrictionalContactProblem::applySlip(NumericVector<Number> & vec_solution,
                                    NumericVector<Number> & ghosted_solution,
                                    std::vector<SlipData> & iterative_slip)
{
  NonlinearSystemBase & nonlinear_sys = getNonlinearSystemBase();
  unsigned int dim = nonlinear_sys.subproblem().mesh().dimension();
  AuxiliarySystem & aux_sys = getAuxiliarySystem();
  NumericVector<Number> & aux_solution = aux_sys.solution();

  MooseVariable * disp_x_var = &getVariable(0, _disp_x);
  MooseVariable * disp_y_var = &getVariable(0, _disp_y);
  MooseVariable * disp_z_var = nullptr;
  MooseVariable * inc_slip_x_var = &getVariable(0, _inc_slip_x);
  MooseVariable * inc_slip_y_var = &getVariable(0, _inc_slip_y);
  MooseVariable * inc_slip_z_var = nullptr;
  if (dim == 3)
  {
    disp_z_var = &getVariable(0, _disp_z);
    inc_slip_z_var = &getVariable(0, _inc_slip_z);
  }

  MooseVariable * disp_var = nullptr;
  MooseVariable * inc_slip_var = nullptr;

  for (unsigned int iislip = 0; iislip < iterative_slip.size(); ++iislip)
  {
    const Node * node = iterative_slip[iislip]._node;
    const unsigned int dof = iterative_slip[iislip]._dof;
    const Real slip = iterative_slip[iislip]._slip;

    if (dof == 0)
    {
      disp_var = disp_x_var;
      inc_slip_var = inc_slip_x_var;
    }
    else if (dof == 1)
    {
      disp_var = disp_y_var;
      inc_slip_var = inc_slip_y_var;
    }
    else
    {
      disp_var = disp_z_var;
      inc_slip_var = inc_slip_z_var;
    }

    dof_id_type solution_dof = node->dof_number(nonlinear_sys.number(), disp_var->number(), 0),
                inc_slip_dof = node->dof_number(aux_sys.number(), inc_slip_var->number(), 0);

    vec_solution.add(solution_dof, slip);
    aux_solution.add(inc_slip_dof, slip);
  }

  aux_solution.close();
  vec_solution.close();
  unsigned int num_slipping_nodes = iterative_slip.size();
  _communicator.sum(num_slipping_nodes);
  if (num_slipping_nodes > 0)
  {
    ghosted_solution = vec_solution;
    ghosted_solution.close();

    updateContactPoints(ghosted_solution, false);

    enforceRateConstraint(vec_solution, ghosted_solution);
  }
}

unsigned int
FrictionalContactProblem::numLocalFrictionalConstraints()
{
  GeometricSearchData & displaced_geom_search_data = getDisplacedProblem()->geomSearchData();
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators =
      &displaced_geom_search_data._penetration_locators;

  unsigned int num_constraints(0);

  for (PLIterator plit = penetration_locators->begin(); plit != penetration_locators->end(); ++plit)
  {
    PenetrationLocator & pen_loc = *plit->second;

    bool frictional_contact_this_interaction = false;

    std::map<std::pair<int, int>, InteractionParams>::iterator ipit;
    std::pair<int, int> ms_pair(pen_loc._master_boundary, pen_loc._slave_boundary);
    ipit = _interaction_params.find(ms_pair);
    if (ipit != _interaction_params.end())
      frictional_contact_this_interaction = true;

    if (frictional_contact_this_interaction)
    {
      std::vector<dof_id_type> & slave_nodes = pen_loc._nearest_node._slave_nodes;

      for (unsigned int i = 0; i < slave_nodes.size(); i++)
      {
        dof_id_type slave_node_num = slave_nodes[i];

        PenetrationInfo * pinfo = pen_loc._penetration_info[slave_node_num];
        if (pinfo)
          if (pinfo->isCaptured())
            ++num_constraints;
      }
    }
  }
  return num_constraints;
}

MooseNonlinearConvergenceReason
FrictionalContactProblem::checkNonlinearConvergence(std::string & msg,
                                                    const PetscInt it,
                                                    const Real xnorm,
                                                    const Real snorm,
                                                    const Real fnorm,
                                                    const Real rtol,
                                                    const Real stol,
                                                    const Real abstol,
                                                    const PetscInt nfuncs,
                                                    const PetscInt /*max_funcs*/,
                                                    const Real ref_resid,
                                                    const Real /*div_threshold*/)
{
  Real my_max_funcs = std::numeric_limits<int>::max();
  Real my_div_threshold = std::numeric_limits<Real>::max();

  MooseNonlinearConvergenceReason reason =
      ReferenceResidualProblem::checkNonlinearConvergence(msg,
                                                          it,
                                                          xnorm,
                                                          snorm,
                                                          fnorm,
                                                          rtol,
                                                          stol,
                                                          abstol,
                                                          nfuncs,
                                                          my_max_funcs,
                                                          ref_resid,
                                                          my_div_threshold);

  _refResidContact = ref_resid; // use initial residual if no reference variables are specified
  updateContactReferenceResidual();

  int min_nl_its_since_contact_update = 1;
  ++_num_nl_its_since_contact_update;

  if ((reason > 0) ||                         // converged
      (reason == MOOSE_NONLINEAR_ITERATING && // iterating and converged within factor
       (fnorm < abstol * _contact_slip_tol_factor ||
        checkConvergenceIndividVars(
            fnorm, abstol * _contact_slip_tol_factor, rtol * _contact_slip_tol_factor, ref_resid))))
  {
    _console << "Slip iteration " << _num_slip_iterations << " ";
    if (_num_slip_iterations < _min_slip_iters)
    { // force another iteration, and do a slip update
      reason = MOOSE_NONLINEAR_ITERATING;
      _do_slip_update = true;
      _console << "Force slip update < min slip iterations" << std::endl;
    }
    else if (_num_slip_iterations < _max_slip_iters)
    { // do a slip update if there is another iteration
      if (_num_nl_its_since_contact_update >= min_nl_its_since_contact_update)
      {
        _do_slip_update = true;

        NonlinearSystemBase & nonlinear_sys = getNonlinearSystemBase();
        nonlinear_sys.update();
        const NumericVector<Number> *& ghosted_solution = nonlinear_sys.currentSolution();

        calculateSlip(*ghosted_solution, nullptr); // Just to calculate slip residual

        if (_slip_residual > _target_contact_residual &&
            _slip_residual > _target_relative_contact_residual * _refResidContact)
        { // force it to keep iterating
          reason = MOOSE_NONLINEAR_ITERATING;
          _console << "Force slip update slip_resid > target: " << _slip_residual << std::endl;
        }
        else
        {
          //_do_slip_update = false; //maybe we want to do this
          _console << "Not forcing slip update slip_resid <= target: " << _slip_residual
                   << std::endl;
        }
      }
      else
      {
        if (_slip_residual > _target_contact_residual &&
            _slip_residual > _target_relative_contact_residual * _refResidContact)
        { // force it to keep iterating
          reason = MOOSE_NONLINEAR_ITERATING;
          _console << "Forcing another nonlinear iteration before slip iteration: "
                   << _num_nl_its_since_contact_update << std::endl;
        }
      }
    }
    else
    { // maxed out
      _console << "Max slip iterations" << std::endl;
      reason = MOOSE_DIVERGED_FUNCTION_COUNT;
    }
  }

  return reason;
}

void
FrictionalContactProblem::updateContactPoints(NumericVector<Number> & ghosted_solution,
                                              bool update_incremental_slip)
{
  GeometricSearchData & displaced_geom_search_data = getDisplacedProblem()->geomSearchData();
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators =
      &displaced_geom_search_data._penetration_locators;

  for (PLIterator plit = penetration_locators->begin(); plit != penetration_locators->end(); ++plit)
  {
    PenetrationLocator & pen_loc = *plit->second;
    pen_loc.setUpdate(true);
  }

  // Do new contact search to update positions of slipped nodes
  _displaced_problem->updateMesh(ghosted_solution, *_aux->currentSolution());

  for (PLIterator plit = penetration_locators->begin(); plit != penetration_locators->end(); ++plit)
  {
    PenetrationLocator & pen_loc = *plit->second;
    pen_loc.setUpdate(false);
  }
  if (update_incremental_slip)
    updateIncrementalSlip();
}

void
FrictionalContactProblem::updateIncrementalSlip()
{
  AuxiliarySystem & aux_sys = getAuxiliarySystem();
  NumericVector<Number> & aux_solution = aux_sys.solution();

  MooseVariable * inc_slip_x_var = &getVariable(0, _inc_slip_x);
  MooseVariable * inc_slip_y_var = &getVariable(0, _inc_slip_y);
  MooseVariable * inc_slip_z_var = nullptr;

  unsigned int dim = getNonlinearSystemBase().subproblem().mesh().dimension();
  if (dim == 3)
    inc_slip_z_var = &getVariable(0, _inc_slip_z);

  GeometricSearchData & displaced_geom_search_data = getDisplacedProblem()->geomSearchData();
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators =
      &displaced_geom_search_data._penetration_locators;

  for (PLIterator plit = penetration_locators->begin(); plit != penetration_locators->end(); ++plit)
  {
    PenetrationLocator & pen_loc = *plit->second;

    bool frictional_contact_this_interaction = false;

    std::map<std::pair<int, int>, InteractionParams>::iterator ipit;
    std::pair<int, int> ms_pair(pen_loc._master_boundary, pen_loc._slave_boundary);
    ipit = _interaction_params.find(ms_pair);
    if (ipit != _interaction_params.end())
      frictional_contact_this_interaction = true;

    if (frictional_contact_this_interaction)
    {
      std::vector<dof_id_type> & slave_nodes = pen_loc._nearest_node._slave_nodes;

      for (unsigned int i = 0; i < slave_nodes.size(); i++)
      {
        dof_id_type slave_node_num = slave_nodes[i];

        if (pen_loc._penetration_info[slave_node_num])
        {
          PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];

          if (info.isCaptured())
          {
            const Node * node = info._node;
            VectorValue<dof_id_type> inc_slip_dofs(
                node->dof_number(aux_sys.number(), inc_slip_x_var->number(), 0),
                node->dof_number(aux_sys.number(), inc_slip_y_var->number(), 0),
                (inc_slip_z_var ? node->dof_number(aux_sys.number(), inc_slip_z_var->number(), 0)
                                : 0));

            RealVectorValue inc_slip = info._incremental_slip;

            for (unsigned int i = 0; i < dim; ++i)
              aux_solution.set(inc_slip_dofs(i), inc_slip(i));
          }
        }
      }
    }
  }

  aux_solution.close();
}
