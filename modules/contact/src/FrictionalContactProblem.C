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
  return params;
}


FrictionalContactProblem::FrictionalContactProblem(const std::string & name, InputParameters params) :
    FEProblem(name, params)
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

  std::map<std::pair<int,int>,InteractionParams>::iterator it;
}

FrictionalContactProblem::~FrictionalContactProblem()
{}

bool
FrictionalContactProblem::shouldUpdateSolution()
{
  return true;
}

bool
FrictionalContactProblem::updateSolution(NumericVector<Number>& vec_solution, const NumericVector<Number>& ghosted_solution)
{
  return slipUpdate(vec_solution, ghosted_solution);
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
  TransientNonlinearImplicitSystem & system = getNonlinearSystem().sys();
//  NonlinearSystem & nlsystem = getNonlinearSystem();

  if(getDisplacedProblem() && _interaction_params.size() > 0)
  {
//    nlsystem._did_slip_update=false;
    computeResidual(system, ghosted_solution, *system.rhs);

    Real max_fraction_of_capacity(0.0);
    int num_contact_nodes(0);
    int num_slipping(0);
    int num_slipped_too_far(0);

    GeometricSearchData & displaced_geom_search_data = getDisplacedProblem()->geomSearchData();
    std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators = &displaced_geom_search_data._penetration_locators;

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
//      if((frictional_contact_this_interaction)&&(nlsystem._do_slip_update))
      {

        InteractionParams & interaction_params = ipit->second;
        Real slip_factor = interaction_params._slip_factor;
        Real friction_coefficient = interaction_params._friction_coefficient;


        std::vector<unsigned int> & slave_nodes = pen_loc._nearest_node._slave_nodes;

        unsigned int slave_boundary = pen_loc._slave_boundary;

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

              AuxiliarySystem & aux_sys = getAuxiliarySystem();

              VectorValue<unsigned int> residual_dofs(node->dof_number(aux_sys.number(), residual_x_var->number(), 0),
                                                      node->dof_number(aux_sys.number(), residual_y_var->number(), 0),
                                                      (residual_z_var ? node->dof_number(nonlinear_sys.number(), residual_z_var->number(), 0) : 0));

              VectorValue<unsigned int> diag_stiff_dofs(node->dof_number(aux_sys.number(), diag_stiff_x_var->number(), 0),
                                                        node->dof_number(aux_sys.number(), diag_stiff_y_var->number(), 0),
                                                        (diag_stiff_z_var ? node->dof_number(nonlinear_sys.number(), diag_stiff_z_var->number(), 0) : 0));

              VectorValue<unsigned int> inc_slip_dofs(node->dof_number(aux_sys.number(), inc_slip_x_var->number(), 0),
                                                      node->dof_number(aux_sys.number(), inc_slip_y_var->number(), 0),
                                                      (inc_slip_z_var ? node->dof_number(nonlinear_sys.number(), inc_slip_z_var->number(), 0) : 0));

              NumericVector<Number> & aux_solution = aux_sys.solution();

              RealVectorValue res_vec;
              RealVectorValue stiff_vec;
              RealVectorValue slip_inc_vec;

              for(unsigned int i=0; i<dim; ++i)
              {
                res_vec(i) = aux_solution(residual_dofs(i));
                stiff_vec(i) = aux_solution(diag_stiff_dofs(i));
                slip_inc_vec(i) = aux_solution(inc_slip_dofs(i));
              }

              RealVectorValue normal_residual = info._normal * (info._normal * res_vec);
              Real normal_force = normal_residual.size();

//              std::cout<<"normal="<<info._normal<<std::endl;
//              std::cout<<"normal_force="<<normal_force<<std::endl;
//              std::cout<<"residual="<<res_vec<<std::endl;
//              std::cout<<"stiffness="<<stiff_vec<<std::endl;

              RealVectorValue tangential_force = normal_residual - res_vec ; // swap sign to make the code more manageable
              Real tangential_force_magnitude = tangential_force.size();

              Real capacity = normal_force * friction_coefficient;
              if(capacity < 0.0)
                capacity = 0.0;

              Real slip_inc = slip_inc_vec.size();
              RealVectorValue slip_iterative(0.0,0.0,0.0);

              bool slipped_too_far = false;
              bool slipping = false;

              if (slip_inc > 0)
              {
                Real slip_dot_tang_force = slip_inc_vec*tangential_force / slip_inc;
                if (slip_dot_tang_force < capacity)
                {
                  slipped_too_far = true;
                  slipping = true;
//                  std::cout<<"STF slip_dot_force: "<<slip_dot_tang_force<<" capacity: "<<capacity<<std::endl;
                }
              }

              if (!slipped_too_far)
              {
                Real fraction_of_capacity = tangential_force_magnitude/capacity;
//                std::cout<<"fraction of capacity="<<fraction_of_capacity<<std::endl;
                if (fraction_of_capacity > max_fraction_of_capacity)
                {
                  max_fraction_of_capacity = fraction_of_capacity;
                }
              }

              Real excess_force = tangential_force_magnitude - capacity;
              if (excess_force < 0.0)
                excess_force = 0;

              if (slipped_too_far)
              {
                RealVectorValue slip_inc_direction = slip_inc_vec / slip_inc;
                Real tangential_force_in_slip_dir = slip_inc_direction*tangential_force;

                RealVectorValue force_from_unit_slip(0.0,0.0,0.0);
                for(unsigned int i=0; i<dim; ++i)
                {
                  force_from_unit_slip(i) = stiff_vec(i) * slip_inc_direction(i);
                }
                Real stiffness_slipdir = force_from_unit_slip * slip_inc_direction; //k=f resolved to slip dir because f=kd and d is a unit vector

                Real slip_distance = slip_factor * (capacity - tangential_force_in_slip_dir) / stiffness_slipdir;
                if (slip_distance < slip_inc)
                {
//                  std::cout<<"STF"<<std::endl;
                  slip_iterative = slip_inc_direction * -slip_distance;
                }
                else
                {
//                  std::cout<<"STF max"<<std::endl;
                  slip_iterative = -slip_inc_vec;
                }
              }
              else if(excess_force > 0)
              {
                slipping = true;
                Real tangential_force_magnitude = tangential_force.size();

                RealVectorValue tangential_direction = tangential_force / tangential_force_magnitude;
                RealVectorValue excess_force_vector = tangential_direction * excess_force;


                for(unsigned int i=0; i<dim; ++i)
                {
                  slip_iterative(i) = slip_factor * excess_force_vector(i) / stiff_vec(i);
                }

                //zero out component of slip_iterative in normal direction
                RealVectorValue slip_iterative_normal_dir = info._normal * (info._normal * slip_iterative);
                slip_iterative = slip_iterative - slip_iterative_normal_dir;
              }
//              std::cout<<"excess f="<<excess_force<<" cap="<<capacity<<std::endl;
//              std::cout<<"tang f  ="<<tangential_force<<std::endl;
//              std::cout<<"it slip ="<<slip_iterative<<std::endl;
//              std::cout<<"inc slip="<<slip_inc_vec<<std::endl;

              if (slipping)
              {
                num_slipping++;
                if (slipped_too_far)
                  num_slipped_too_far++;
                for(unsigned int i=0; i<dim; ++i)
                {
                  vec_solution.add(solution_dofs(i), slip_iterative(i));
                  aux_solution.add(inc_slip_dofs(i), slip_iterative(i));
                }
              }

              aux_solution.close();
            }
          }
        }

      }
      vec_solution.close(); //does this do anything if it's not a ghosted vector?
    }

    Parallel::sum(num_contact_nodes);
    Parallel::sum(num_slipping);
    Parallel::sum(num_slipped_too_far);
    Parallel::max(max_fraction_of_capacity);

    std::cout<<"Num contact nodes: "<<num_contact_nodes<<std::endl;
    std::cout<<"Num slipping: "<<num_slipping<<std::endl;
    std::cout<<"Num slipped too far: "<<num_slipped_too_far<<std::endl;
    std::cout<<"Max fraction of capacity: "<<max_fraction_of_capacity<<std::endl;
//    nlsystem._did_slip_update=true;
    if (num_slipping > 0)
      updatedSolution = true;
  }
//  if (nlsystem._do_slip_update)
//  {
//    nlsystem._do_slip_update = false;
//  }

  return updatedSolution;
}
