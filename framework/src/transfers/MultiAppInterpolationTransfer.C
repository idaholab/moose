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

#define NOTFOUND -999999

#include "MultiAppInterpolationTransfer.h"

// Moose
#include "MooseTypes.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"

// libMesh
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"
#include "libmesh/radial_basis_interpolation.h"

template<>
InputParameters validParams<MultiAppInterpolationTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();

  params.addRequiredParam<AuxVariableName>("variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  params.addParam<bool>("displaced_source_mesh", false, "Whether or not to use the displaced mesh for the source mesh.");
  params.addParam<bool>("displaced_target_mesh", false, "Whether or not to use the displaced mesh for the target mesh.");

  params.addParam<unsigned int>("num_points", 3, "The number of nearest points to use for interpolation.");
  params.addParam<Real>("power", 2, "The polynomial power to use for calculation of the decay in the interpolation.");

  MooseEnum interp_type("inverse_distance, radial_basis", "inverse_distance");
  params.addParam<MooseEnum>("interp_type", interp_type, "The algorithm to use for interpolation.");

  params.addParam<Real>("radius", -1, "Radius to use for radial_basis interpolation.  If negative then the radius is taken as the max distance between points.");

  return params;
}

MultiAppInterpolationTransfer::MultiAppInterpolationTransfer(const std::string & name, InputParameters parameters) :
    MultiAppTransfer(name, parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable")),
    _displaced_source_mesh(getParam<bool>("displaced_source_mesh")),
    _displaced_target_mesh(getParam<bool>("displaced_target_mesh")),
    _num_points(getParam<unsigned int>("num_points")),
    _power(getParam<Real>("power")),
    _interp_type(getParam<MooseEnum>("interp_type")),
    _radius(getParam<Real>("radius"))
{
  // This transfer does not work with ParallelMesh
  _fe_problem.mesh().errorIfParallelDistribution("MultiAppInterpolationTransfer");
}

void
MultiAppInterpolationTransfer::execute()
{
  Moose::out << "Beginning InterpolationTransfer " << _name << std::endl;

  switch(_direction)
  {
    case TO_MULTIAPP:
    {
      FEProblem & from_problem = *_multi_app->problem();
      MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);

      MeshBase * from_mesh = NULL;

      if(_displaced_source_mesh && from_problem.getDisplacedProblem())
        from_mesh = &from_problem.getDisplacedProblem()->mesh().getMesh();
      else
        from_mesh = &from_problem.mesh().getMesh();

      SystemBase & from_system_base = from_var.sys();
      System & from_sys = from_system_base.system();

      unsigned int from_sys_num = from_sys.number();
      unsigned int from_var_num = from_sys.variable_number(from_var.name());

      bool from_is_nodal = from_sys.variable_type(from_var_num).family == LAGRANGE;

      // EquationSystems & from_es = from_sys.get_equation_systems();

      NumericVector<Number> & from_solution = *from_sys.solution;

      InverseDistanceInterpolation<LIBMESH_DIM> * idi;

      switch(_interp_type)
      {
        case 0:
          idi = new InverseDistanceInterpolation<LIBMESH_DIM>(libMesh::CommWorld, _num_points, _power);
          break;
        case 1:
          idi = new RadialBasisInterpolation<LIBMESH_DIM>(libMesh::CommWorld, _radius);
          break;
        default:
          mooseError("Unknown interpolation type!");
      }

      std::vector<Point>  &src_pts  (idi->get_source_points());
      std::vector<Number> &src_vals (idi->get_source_vals());

      std::vector<std::string> field_vars;
      field_vars.push_back(_to_var_name);
      idi->set_field_variables(field_vars);

      std::vector<std::string> vars;
      vars.push_back(_to_var_name);

      if(from_is_nodal)
      {
        MeshBase::const_node_iterator from_nodes_it    = from_mesh->local_nodes_begin();
        MeshBase::const_node_iterator from_nodes_end   = from_mesh->local_nodes_end();

        for(; from_nodes_it != from_nodes_end; ++from_nodes_it)
        {
          Node * from_node = *from_nodes_it;

          // Assuming LAGRANGE!
          dof_id_type from_dof = from_node->dof_number(from_sys_num, from_var_num, 0);

          src_pts.push_back(*from_node);
          src_vals.push_back(from_solution(from_dof));
        }
      }
      else
      {
        MeshBase::const_element_iterator from_elements_it    = from_mesh->local_elements_begin();
        MeshBase::const_element_iterator from_elements_end   = from_mesh->local_elements_end();

        for(; from_elements_it != from_elements_end; ++from_elements_it)
        {
          Elem * from_elem = *from_elements_it;

          // Assuming CONSTANT MONOMIAL
          dof_id_type from_dof = from_elem->dof_number(from_sys_num, from_var_num, 0);

          src_pts.push_back(from_elem->centroid());
          src_vals.push_back(from_solution(from_dof));
        }
      }

      // We have only set local values - prepare for use by gathering remote gata
      idi->prepare_for_use();

      for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
      {
        if(_multi_app->hasLocalApp(i))
        {
          MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

          // Loop over the master nodes and set the value of the variable
          System * to_sys = find_sys(_multi_app->appProblem(i)->es(), _to_var_name);

          if(!to_sys)
            mooseError("Cannot find variable "<<_to_var_name<<" for "<<_name<<" Transfer");

          unsigned int sys_num = to_sys->number();
          unsigned int var_num = to_sys->variable_number(_to_var_name);
          NumericVector<Real> & solution = _multi_app->appTransferVector(i, _to_var_name);

          MeshBase * mesh = NULL;

          if(_displaced_target_mesh && _multi_app->appProblem(i)->getDisplacedProblem())
            mesh = &_multi_app->appProblem(i)->getDisplacedProblem()->mesh().getMesh();
          else
            mesh = &_multi_app->appProblem(i)->mesh().getMesh();

          bool is_nodal = to_sys->variable_type(var_num).family == LAGRANGE;

          if(is_nodal)
          {
            MeshBase::const_node_iterator node_it = mesh->local_nodes_begin();
            MeshBase::const_node_iterator node_end = mesh->local_nodes_end();

            for(; node_it != node_end; ++node_it)
            {
              Node * node = *node_it;

              Point actual_position = *node+_multi_app->position(i);

              if(node->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this node
              {
                std::vector<Point> pts;
                std::vector<Number> vals;

                pts.push_back(actual_position);
                vals.resize(1);

                idi->interpolate_field_data(vars, pts, vals);

                Real value = vals.front();

                // The zero only works for LAGRANGE!
                dof_id_type dof = node->dof_number(sys_num, var_num, 0);

                solution.set(dof, value);
              }
            }
          }
          else // Elemental
          {
            MeshBase::const_element_iterator elem_it = mesh->local_elements_begin();
            MeshBase::const_element_iterator elem_end = mesh->local_elements_end();

            for(; elem_it != elem_end; ++elem_it)
            {
              Elem * elem = *elem_it;

              Point centroid = elem->centroid();
              Point actual_position = centroid+_multi_app->position(i);

              if(elem->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this elem
              {
                std::vector<Point> pts;
                std::vector<Number> vals;

                pts.push_back(actual_position);
                vals.resize(1);

                idi->interpolate_field_data(vars, pts, vals);

                Real value = vals.front();

                dof_id_type dof = elem->dof_number(sys_num, var_num, 0);

                solution.set(dof, value);
              }
            }
          }

          solution.close();
          to_sys->update();

          // Swap back
          Moose::swapLibMeshComm(swapped);
        }
      }

      delete idi;

      break;
    }
    case FROM_MULTIAPP:
    {
      FEProblem & to_problem = *_multi_app->problem();
      MooseVariable & to_var = to_problem.getVariable(0, _to_var_name);
      SystemBase & to_system_base = to_var.sys();

      System & to_sys = to_system_base.system();

      NumericVector<Real> & to_solution = *to_sys.solution;

      unsigned int to_sys_num = to_sys.number();

      // Only works with a serialized mesh to transfer to!
      mooseAssert(to_sys.get_mesh().is_serial(), "MultiAppInterpolationTransfer only works with SerialMesh!");

      unsigned int to_var_num = to_sys.variable_number(to_var.name());

      // EquationSystems & to_es = to_sys.get_equation_systems();

      MeshBase * to_mesh = NULL;

      if(_displaced_target_mesh && to_problem.getDisplacedProblem())
        to_mesh = &to_problem.getDisplacedProblem()->mesh().getMesh();
      else
        to_mesh = &to_problem.mesh().getMesh();

      bool is_nodal = to_sys.variable_type(to_var_num).family == LAGRANGE;

      InverseDistanceInterpolation<LIBMESH_DIM> * idi;

      switch(_interp_type)
      {
        case 0:
          idi = new InverseDistanceInterpolation<LIBMESH_DIM>(libMesh::CommWorld, _num_points, _power);
          break;
        case 1:
          idi = new RadialBasisInterpolation<LIBMESH_DIM>(libMesh::CommWorld, _radius);
          break;
        default:
          mooseError("Unknown interpolation type!");
      }

      std::vector<Point>  &src_pts  (idi->get_source_points());
      std::vector<Number> &src_vals (idi->get_source_vals());

      std::vector<std::string> field_vars;
      field_vars.push_back(_to_var_name);
      idi->set_field_variables(field_vars);

      std::vector<std::string> vars;
      vars.push_back(_to_var_name);

      for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
      {
        if(!_multi_app->hasLocalApp(i))
          continue;

        MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

        FEProblem & from_problem = *_multi_app->appProblem(i);
        MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
        SystemBase & from_system_base = from_var.sys();

        System & from_sys = from_system_base.system();
        unsigned int from_sys_num = from_sys.number();

        unsigned int from_var_num = from_sys.variable_number(from_var.name());

        bool from_is_nodal = from_sys.variable_type(from_var_num).family == LAGRANGE;

        // EquationSystems & from_es = from_sys.get_equation_systems();

        NumericVector<Number> & from_solution = *from_sys.solution;

        MeshBase * from_mesh = NULL;

        if(_displaced_source_mesh && from_problem.getDisplacedProblem())
          from_mesh = &from_problem.getDisplacedProblem()->mesh().getMesh();
        else
          from_mesh = &from_problem.mesh().getMesh();

        Point app_position = _multi_app->position(i);

        if(from_is_nodal)
        {
          MeshBase::const_node_iterator from_nodes_it    = from_mesh->local_nodes_begin();
          MeshBase::const_node_iterator from_nodes_end   = from_mesh->local_nodes_end();

          for(; from_nodes_it != from_nodes_end; ++from_nodes_it)
          {
            Node * from_node = *from_nodes_it;

            // Assuming LAGRANGE!
            dof_id_type from_dof = from_node->dof_number(from_sys_num, from_var_num, 0);

            src_pts.push_back(*from_node+app_position);
            src_vals.push_back(from_solution(from_dof));
          }
        }
        else
        {
          MeshBase::const_element_iterator from_elements_it    = from_mesh->local_elements_begin();
          MeshBase::const_element_iterator from_elements_end   = from_mesh->local_elements_end();

          for(; from_elements_it != from_elements_end; ++from_elements_it)
          {
            Elem * from_element = *from_elements_it;

            // Assuming LAGRANGE!
            dof_id_type from_dof = from_element->dof_number(from_sys_num, from_var_num, 0);

            src_pts.push_back(from_element->centroid()+app_position);
            src_vals.push_back(from_solution(from_dof));
          }
        }

        Moose::swapLibMeshComm(swapped);
      }

      // We have only set local values - prepare for use by gathering remote gata
      idi->prepare_for_use();

      // Now do the interpolation to the target system
      if(is_nodal)
      {
        MeshBase::const_node_iterator node_it = to_mesh->local_nodes_begin();
        MeshBase::const_node_iterator node_end = to_mesh->local_nodes_end();

        for(; node_it != node_end; ++node_it)
        {
          Node * node = *node_it;

          if(node->n_dofs(to_sys_num, to_var_num) > 0) // If this variable has dofs at this node
          {
            std::vector<Point> pts;
            std::vector<Number> vals;

            pts.push_back(*node);
            vals.resize(1);

            idi->interpolate_field_data(vars, pts, vals);

            Real value = vals.front();

            // The zero only works for LAGRANGE!
            dof_id_type dof = node->dof_number(to_sys_num, to_var_num, 0);

            to_solution.set(dof, value);
          }
        }
      }
      else // Elemental
      {
        MeshBase::const_element_iterator elem_it = to_mesh->local_elements_begin();
        MeshBase::const_element_iterator elem_end = to_mesh->local_elements_end();

        for(; elem_it != elem_end; ++elem_it)
        {
          Elem * elem = *elem_it;

          Point centroid = elem->centroid();

          if(elem->n_dofs(to_sys_num, to_var_num) > 0) // If this variable has dofs at this elem
          {
            std::vector<Point> pts;
            std::vector<Number> vals;

            pts.push_back(centroid);
            vals.resize(1);

            idi->interpolate_field_data(vars, pts, vals);

            Real value = vals.front();

            dof_id_type dof = elem->dof_number(to_sys_num, to_var_num, 0);

            to_solution.set(dof, value);
          }
        }
      }

      to_solution.close();
      to_sys.update();

      delete idi;

      break;
    }
  }

  Moose::out << "Finished InterpolationTransfer " << _name << std::endl;
}

Node * MultiAppInterpolationTransfer::getNearestNode(const Point & p, Real & distance, const MeshBase::const_node_iterator & nodes_begin, const MeshBase::const_node_iterator & nodes_end)
{
  distance = std::numeric_limits<Real>::max();
  Node * nearest = NULL;

  for(MeshBase::const_node_iterator node_it = nodes_begin; node_it != nodes_end; ++node_it)
  {
    Node & node = *(*node_it);
    Real current_distance = (p-node).size();

    if(current_distance < distance)
    {
      distance = current_distance;
      nearest = &node;
    }
  }

  return nearest;
}

