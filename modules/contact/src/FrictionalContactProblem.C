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

#include "FrictionalContactProblem.h"

#include "NonlinearSystem.h"
#include "DisplacedProblem.h"
#include "PenetrationLocator.h"
#include "NearestNodeLocator.h"
#include "MooseApp.h"

template<>
InputParameters validParams<FrictionalContactProblem>()
{
  InputParameters params = validParams<FEProblem>();
  params.addRequiredParam<std::vector<int> >("master","IDs of the master surfaces for which the slip should be calculated");
  params.addRequiredParam<std::vector<int> >("slave","IDs of the slave surfaces for which the slip should be calculated");
  params.addRequiredParam<std::vector<Real> >("friction_coefficient","Coefficient of friction for sliding contact for each interaction");
  params.addRequiredParam<std::vector<Real> >("slip_factor","Fraction of calculated slip to be applied for each interaction");
  params.addRequiredParam<NonlinearVariableName>("disp_x","Variable containing the x displacement");
  params.addRequiredParam<NonlinearVariableName>("disp_y","Variable containing the y displacement");
  params.addParam<NonlinearVariableName>        ("disp_z","Variable containing the z displacement");
  params.addRequiredParam<AuxVariableName>("residual_x","Auxiliary variable containing the saved x residual");
  params.addRequiredParam<AuxVariableName>("residual_y","Auxiliary variable containing the saved y residual");
  params.addParam<AuxVariableName>        ("residual_z","Auxiliary variable containing the saved z residual");
  params.addRequiredParam<AuxVariableName>("diag_stiff_x","Auxiliary variable containing the saved x diagonal stiffness");
  params.addRequiredParam<AuxVariableName>("diag_stiff_y","Auxiliary variable containing the saved y diagonal stiffness");
  params.addParam<AuxVariableName>        ("diag_stiff_z","Auxiliary variable containing the saved z diagonal stiffness");
  params.addRequiredParam<AuxVariableName>("inc_slip_x","Auxiliary variable to store the x incremental slip");
  params.addRequiredParam<AuxVariableName>("inc_slip_y","Auxiliary variable to store the y incremental slip");
  params.addParam<AuxVariableName>        ("inc_slip_z","Auxiliary variable to store the z incremental slip");
  params.addParam<int>         ("minimum_slip_iterations",1,"Minimum number of slip iterations per step");
  params.addParam<int>         ("maximum_slip_iterations",100,"Maximum number of slip iterations per step");
  params.addParam<int>         ("slip_updates_per_iteration",1,"Number of slip updates per contact iteration");
  params.addRequiredParam<Real>("target_contact_residual","Frictional contact residual convergence criterion");
  params.addParam<Real>        ("contact_slip_tolerance_factor",10.0,"Multiplier on convergence criteria to determine when to start slipping");
  return params;
}

FrictionalContactProblem::FrictionalContactProblem(const std::string & name, InputParameters params) :
    FEProblem(name, params),
    _slip_residual(0.0),
    _do_slip_update(false),
    _num_slip_iterations(0)
{
  Moose::app->parser().extractParams("Problem", params);
  params.checkParams("Problem");

  std::vector<int> master = params.get<std::vector<int> >("master");
  std::vector<int> slave = params.get<std::vector<int> >("slave");
  std::vector<Real> friction_coefficient = params.get<std::vector<Real> >("friction_coefficient");
  std::vector<Real> slip_factor = params.get<std::vector<Real> >("slip_factor");

  unsigned int dim = getNonlinearSystem().subproblem().mesh().dimension();

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
    mooseError("Sizes of master surface and slave surface lists must match in FrictionalContactProblem");
  if (num_interactions != friction_coefficient.size())
    mooseError("Must have friction coefficient defined for every interaction in FrictionalContactProblem");
  if (num_interactions != slip_factor.size())
    mooseError("Must have slip factor defined for every interaction in FrictionalContactProblem");

  for (unsigned int i=0; i<master.size(); ++i)
  {
    std::pair<int,int> ms_pair(master[i],slave[i]);
    InteractionParams ip;
    ip._friction_coefficient = friction_coefficient[i];
    ip._slip_factor = slip_factor[i];

    _interaction_params[ms_pair] = ip;
  }

  _min_slip_iters = params.get<int>("minimum_slip_iterations");
  _max_slip_iters = params.get<int>("maximum_slip_iterations");
  _slip_updates_per_iter = params.get<int>("slip_updates_per_iteration");
  _target_contact_residual = params.get<Real>("target_contact_residual");
  _contact_slip_tol_factor = params.get<Real>("contact_slip_tolerance_factor");
}

FrictionalContactProblem::~FrictionalContactProblem()
{}

void
FrictionalContactProblem::timestepSetup()
{
  _do_slip_update = false;
  _num_slip_iterations = 0;
  FEProblem::timestepSetup();
}

bool
FrictionalContactProblem::shouldUpdateSolution()
{
  return true;
}

bool
FrictionalContactProblem::updateSolution(NumericVector<Number>& vec_solution, NumericVector<Number>& ghosted_solution)
{
  bool slipped_nodes(false);
  if (_do_slip_update)
  {
    for (unsigned i=0; i<_slip_updates_per_iter; i++)
    {
      if (i>0)
      {
        ghosted_solution = vec_solution;
        ghosted_solution.close();
      }

      bool updated_this_iter = slipUpdate(vec_solution, ghosted_solution);
      slipped_nodes |= updated_this_iter;
      if (!updated_this_iter)
        break;
    }
  }
  _num_slip_iterations++;
  _do_slip_update = false;
  return slipped_nodes;
}

bool
FrictionalContactProblem::slipUpdate(NumericVector<Number>& vec_solution, const NumericVector<Number>& ghosted_solution)
{
  NonlinearSystem & nonlinear_sys = getNonlinearSystem();
  unsigned int dim = nonlinear_sys.subproblem().mesh().dimension();

  MooseVariable * disp_x_var = &getVariable(0,_disp_x);
  MooseVariable * disp_y_var = &getVariable(0,_disp_y);
  MooseVariable * disp_z_var = NULL;
  MooseVariable * residual_x_var = &getVariable(0,_residual_x);
  MooseVariable * residual_y_var = &getVariable(0,_residual_y);
  MooseVariable * residual_z_var = NULL;
  MooseVariable * diag_stiff_x_var = &getVariable(0,_diag_stiff_x);
  MooseVariable * diag_stiff_y_var = &getVariable(0,_diag_stiff_y);
  MooseVariable * diag_stiff_z_var = NULL;
  MooseVariable * inc_slip_x_var = &getVariable(0,_inc_slip_x);
  MooseVariable * inc_slip_y_var = &getVariable(0,_inc_slip_y);
  MooseVariable * inc_slip_z_var = NULL;
  if (dim == 3)
  {
    disp_z_var = &getVariable(0,_disp_z);
    residual_z_var = &getVariable(0,_residual_z);
    diag_stiff_z_var = &getVariable(0,_diag_stiff_z);
    inc_slip_z_var = &getVariable(0,_inc_slip_z);
  }

  bool updatedSolution = false;
  _slip_residual = 0.0;
  TransientNonlinearImplicitSystem & system = getNonlinearSystem().sys();

  if(getDisplacedProblem() && _interaction_params.size() > 0)
  {
    computeResidual(system, ghosted_solution, *system.rhs);

    int num_contact_nodes(0);
    int num_slipping(0);
    int num_slipped_too_far(0);

    GeometricSearchData & displaced_geom_search_data = getDisplacedProblem()->geomSearchData();
    std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators = &displaced_geom_search_data._penetration_locators;

    AuxiliarySystem & aux_sys = getAuxiliarySystem();
    NumericVector<Number> & aux_solution = aux_sys.solution();

    for(std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator plit = penetration_locators->begin();
      plit != penetration_locators->end();
      ++plit)
    {
      PenetrationLocator & pen_loc = *plit->second;
      std::map<unsigned int, bool> & has_penetrated = pen_loc._has_penetrated;

      bool frictional_contact_this_interaction = false;

      std::map<std::pair<int,int>,InteractionParams>::iterator ipit;
      std::pair<int,int> ms_pair(pen_loc._master_boundary,pen_loc._slave_boundary);
      ipit = _interaction_params.find(ms_pair);
      if (ipit != _interaction_params.end())
        frictional_contact_this_interaction = true;

      if(frictional_contact_this_interaction)
      {

        InteractionParams & interaction_params = ipit->second;
        Real slip_factor = interaction_params._slip_factor;
        Real friction_coefficient = interaction_params._friction_coefficient;


        std::vector<unsigned int> & slave_nodes = pen_loc._nearest_node._slave_nodes;

        for(unsigned int i=0; i<slave_nodes.size(); i++)
        {
          unsigned int slave_node_num = slave_nodes[i];

          if(pen_loc._penetration_info[slave_node_num])
          {
            PenetrationLocator::PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];


            std::map<unsigned int, bool>::iterator hpit( has_penetrated.find( slave_node_num ) );

            if(hpit != has_penetrated.end())
            {
//              std::cout<<"Slave node: "<<slave_node_num<<std::endl;
              num_contact_nodes++;
              const Node * node = info._node;

              VectorValue<unsigned int> solution_dofs(node->dof_number(nonlinear_sys.number(), disp_x_var->number(), 0),
                                                      node->dof_number(nonlinear_sys.number(), disp_y_var->number(), 0),
                                                      (disp_z_var ? node->dof_number(nonlinear_sys.number(), disp_z_var->number(), 0) : 0));

              VectorValue<unsigned int> residual_dofs(node->dof_number(aux_sys.number(), residual_x_var->number(), 0),
                                                      node->dof_number(aux_sys.number(), residual_y_var->number(), 0),
                                                      (residual_z_var ? node->dof_number(nonlinear_sys.number(), residual_z_var->number(), 0) : 0));

              VectorValue<unsigned int> diag_stiff_dofs(node->dof_number(aux_sys.number(), diag_stiff_x_var->number(), 0),
                                                        node->dof_number(aux_sys.number(), diag_stiff_y_var->number(), 0),
                                                        (diag_stiff_z_var ? node->dof_number(nonlinear_sys.number(), diag_stiff_z_var->number(), 0) : 0));

              VectorValue<unsigned int> inc_slip_dofs(node->dof_number(aux_sys.number(), inc_slip_x_var->number(), 0),
                                                      node->dof_number(aux_sys.number(), inc_slip_y_var->number(), 0),
                                                      (inc_slip_z_var ? node->dof_number(nonlinear_sys.number(), inc_slip_z_var->number(), 0) : 0));

              RealVectorValue res_vec;
              RealVectorValue stiff_vec;
              RealVectorValue slip_inc_vec;

              for(unsigned int i=0; i<dim; ++i)
              {
                res_vec(i) = aux_solution(residual_dofs(i));
                stiff_vec(i) = aux_solution(diag_stiff_dofs(i));
                slip_inc_vec(i) = aux_solution(inc_slip_dofs(i));
              }

              RealVectorValue slip_iterative(0.0,0.0,0.0);
              Real interaction_slip_residual = 0.0;
              ContactState state = calculateSlip(slip_iterative, interaction_slip_residual, info._normal, res_vec, slip_inc_vec, stiff_vec, friction_coefficient, slip_factor, dim);
              _slip_residual += interaction_slip_residual*interaction_slip_residual;

              if (state == SLIPPING || state == SLIPPED_TOO_FAR)
              {
                num_slipping++;
                if (state == SLIPPED_TOO_FAR)
                  num_slipped_too_far++;
                for(unsigned int i=0; i<dim; ++i)
                {
                  vec_solution.add(solution_dofs(i), slip_iterative(i));
                  aux_solution.add(inc_slip_dofs(i), slip_iterative(i));
                }
              }

            }
          }
        }

      }
    }

    aux_solution.close();
    vec_solution.close();

    Parallel::sum(num_contact_nodes);
    Parallel::sum(num_slipping);
    Parallel::sum(num_slipped_too_far);
    Parallel::sum(_slip_residual);
    _slip_residual = std::sqrt(_slip_residual);

    std::cout<<"Num contact nodes: "<<num_contact_nodes<<std::endl;
    std::cout<<"Num slipping: "<<num_slipping<<std::endl;
    std::cout<<"Num slipped too far: "<<num_slipped_too_far<<std::endl;
    std::cout<<"Slip residual: "<<_slip_residual<<std::endl;
    if (num_slipping > 0)
      updatedSolution = true;
  }

  return updatedSolution;
}

ContactState
FrictionalContactProblem::calculateSlip(RealVectorValue &slip,
                                        Real &slip_residual,
                                        const RealVectorValue &normal,
                                        const RealVectorValue &residual,
                                        const RealVectorValue &incremental_slip,
                                        const RealVectorValue &stiffness,
                                        const Real friction_coefficient,
                                        const Real slip_factor,
                                        const int dim)
{

  ContactState state = STICKING;

  RealVectorValue normal_residual = normal * (normal * residual);
  Real normal_force = normal_residual.size();

//  std::cout<<"normal="<<info._normal<<std::endl;
//  std::cout<<"normal_force="<<normal_force<<std::endl;
//  std::cout<<"residual="<<residual<<std::endl;
//  std::cout<<"stiffness="<<stiff_vec<<std::endl;

  RealVectorValue tangential_force = normal_residual - residual ; // swap sign to make the code more manageable
  Real tangential_force_magnitude = tangential_force.size();

  Real capacity = normal_force * friction_coefficient;
  if(capacity < 0.0)
    capacity = 0.0;

  Real slip_inc = incremental_slip.size();

  slip(0)=0.0;
  slip(1)=0.0;
  slip(2)=0.0;

  if (slip_inc > 0)
  {
    state = SLIPPING;
    Real slip_dot_tang_force = incremental_slip*tangential_force / slip_inc;
    if (slip_dot_tang_force < capacity)
    {
      state = SLIPPED_TOO_FAR;
//      std::cout<<"STF slip_dot_force: "<<slip_dot_tang_force<<" capacity: "<<capacity<<std::endl;
    }
  }

  Real excess_force = tangential_force_magnitude - capacity;
  if (excess_force < 0.0)
    excess_force = 0;

  if (state == SLIPPED_TOO_FAR)
  {
    RealVectorValue slip_inc_direction = incremental_slip / slip_inc;
    Real tangential_force_in_slip_dir = slip_inc_direction*tangential_force;
    slip_residual = capacity - tangential_force_in_slip_dir;

    RealVectorValue force_from_unit_slip(0.0,0.0,0.0);
    for(int i=0; i<dim; ++i)
    {
      force_from_unit_slip(i) = stiffness(i) * slip_inc_direction(i);
    }
    Real stiffness_slipdir = force_from_unit_slip * slip_inc_direction; //k=f resolved to slip dir because f=kd and d is a unit vector

    Real slip_distance = slip_factor * (capacity - tangential_force_in_slip_dir) / stiffness_slipdir;
    if (slip_distance < slip_inc)
    {
//      std::cout<<"STF"<<std::endl;
      slip = slip_inc_direction * -slip_distance;
    }
    else
    {
//      std::cout<<"STF max"<<std::endl;
      slip = -incremental_slip;
    }
  }
  else if(excess_force > 0)
  {
    state = SLIPPING;
    Real tangential_force_magnitude = tangential_force.size();
    slip_residual = excess_force;

    RealVectorValue tangential_direction = tangential_force / tangential_force_magnitude;
    RealVectorValue excess_force_vector = tangential_direction * excess_force;


    for(int i=0; i<dim; ++i)
    {
      slip(i) = slip_factor * excess_force_vector(i) / stiffness(i);
    }

    //zero out component of slip in normal direction
    RealVectorValue slip_normal_dir = normal * (normal * slip);
    slip = slip - slip_normal_dir;
  }
  return state;
}

MooseNonlinearConvergenceReason
FrictionalContactProblem::checkNonlinearConvergence(std::string &msg, const int it, const Real xnorm, const Real snorm, const Real fnorm, Real &ttol, const Real rtol, const Real stol, const Real abstol, const int nfuncs, const int max_funcs)
{
  MooseNonlinearConvergenceReason reason = FEProblem::checkNonlinearConvergence(msg, it, xnorm, snorm, fnorm, ttol, rtol, stol, abstol, nfuncs, max_funcs);

  if (reason >= 0) //converged or iterating
  {
    if (fnorm < abstol*_contact_slip_tol_factor || fnorm <= ttol*_contact_slip_tol_factor)
    {
      std::cout<<"Slip iteration "<<_num_slip_iterations<<" ";
      if (_num_slip_iterations < _min_slip_iters)
      { //force another iteration, and do a slip update
        reason = MOOSE_ITERATING;
        _do_slip_update = true;
        std::cout<<"Force slip update < min iterations"<<std::endl;
      }
      else if (_num_slip_iterations < _max_slip_iters)
      { //do a slip update if there is another iteration
        _do_slip_update = true;

        if (_slip_residual > _target_contact_residual)
        { //force it to keep iterating
          reason = MOOSE_ITERATING;
          std::cout<<"Force slip update slip_resid > target"<<std::endl;
        }
        else
          std::cout<<"Not forcing slip update"<<std::endl;
      }
      else
      { //maxed out
        std::cout<<"Max slip iterations"<<std::endl;
        reason = MOOSE_DIVERGED_FUNCTION_COUNT;
      }
    }
  }

  return(reason);
}
