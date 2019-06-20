//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppInterpolationTransfer.h"

// MOOSE includes
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "MultiApp.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"
#include "libmesh/radial_basis_interpolation.h"

registerMooseObject("MooseApp", MultiAppInterpolationTransfer);

template <>
InputParameters
validParams<MultiAppInterpolationTransfer>()
{
  InputParameters params = validParams<MultiAppFieldTransfer>();
  params.addClassDescription(
      "Transfers the value to the target domain from the nearest node in the source domain.");
  params.addParam<unsigned int>(
      "num_points", 3, "The number of nearest points to use for interpolation.");
  params.addParam<Real>(
      "power", 2, "The polynomial power to use for calculation of the decay in the interpolation.");

  MooseEnum interp_type("inverse_distance radial_basis", "inverse_distance");
  params.addParam<MooseEnum>("interp_type", interp_type, "The algorithm to use for interpolation.");

  params.addParam<Real>("radius",
                        -1,
                        "Radius to use for radial_basis interpolation.  If negative "
                        "then the radius is taken as the max distance between "
                        "points.");

  return params;
}

MultiAppInterpolationTransfer::MultiAppInterpolationTransfer(const InputParameters & parameters)
  : MultiAppFieldTransfer(parameters),
    _num_points(getParam<unsigned int>("num_points")),
    _power(getParam<Real>("power")),
    _interp_type(getParam<MooseEnum>("interp_type")),
    _radius(getParam<Real>("radius"))
{
  // This transfer does not work with DistributedMesh
  _fe_problem.mesh().errorIfDistributedMesh("MultiAppInterpolationTransfer");

  if (_to_var_names.size() != 1)
    paramError("variable", " Support single to-variable only ");

  if (_from_var_names.size() != 1)
    paramError("source_variable", " Support single from-variable only ");
}

void
MultiAppInterpolationTransfer::execute()
{
  _console << "Beginning InterpolationTransfer " << name() << std::endl;

  switch (_direction)
  {
    case TO_MULTIAPP:
    {
      FEProblemBase & from_problem = _multi_app->problemBase();
      MooseVariableFEBase & from_var = from_problem.getVariable(
          0, _from_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);

      MeshBase * from_mesh = NULL;

      if (_displaced_source_mesh && from_problem.getDisplacedProblem())
        from_mesh = &from_problem.getDisplacedProblem()->mesh().getMesh();
      else
        from_mesh = &from_problem.mesh().getMesh();

      SystemBase & from_system_base = from_var.sys();
      System & from_sys = from_system_base.system();

      unsigned int from_sys_num = from_sys.number();
      unsigned int from_var_num = from_sys.variable_number(from_var.name());

      auto & fe_type = from_sys.variable_type(from_var_num);
      bool from_is_constant = fe_type.order == CONSTANT;
      bool from_is_nodal = fe_type.family == LAGRANGE;

      if (fe_type.order > FIRST && !from_is_nodal)
        mooseError("We don't currently support second order or higher elemental variable ");

      // EquationSystems & from_es = from_sys.get_equation_systems();

      NumericVector<Number> & from_solution = *from_sys.solution;

      InverseDistanceInterpolation<LIBMESH_DIM> * idi;

      switch (_interp_type)
      {
        case 0:
          idi = new InverseDistanceInterpolation<LIBMESH_DIM>(from_sys.comm(), _num_points, _power);
          break;
        case 1:
          idi = new RadialBasisInterpolation<LIBMESH_DIM>(from_sys.comm(), _radius);
          break;
        default:
          mooseError("Unknown interpolation type!");
      }

      std::vector<Point> & src_pts(idi->get_source_points());
      std::vector<Number> & src_vals(idi->get_source_vals());

      std::vector<std::string> field_vars;
      field_vars.push_back(_to_var_name);
      idi->set_field_variables(field_vars);

      std::vector<std::string> vars;
      vars.push_back(_to_var_name);

      if (from_is_nodal)
      {
        for (const auto & from_node : from_mesh->local_node_ptr_range())
        {
          // Assuming LAGRANGE!
          if (from_node->n_comp(from_sys_num, from_var_num) == 0)
            continue;

          dof_id_type from_dof = from_node->dof_number(from_sys_num, from_var_num, 0);

          src_pts.push_back(*from_node);
          src_vals.push_back(from_solution(from_dof));
        }
      }
      else
      {
        std::vector<Point> points;
        for (const auto & from_elem :
             as_range(from_mesh->local_elements_begin(), from_mesh->local_elements_end()))
        {
          // Skip this element if the variable has no dofs at it.
          if (from_elem->n_dofs(from_sys_num, from_var_num) < 1)
            continue;

          points.clear();
          if (from_is_constant)
            points.push_back(from_elem->centroid());
          else
            for (auto & node : from_elem->node_ref_range())
              points.push_back(node);

          unsigned int n_comp = from_elem->n_comp(from_sys_num, from_var_num);
          auto n_points = points.size();
          // We assume each point corresponds to one component of elemental variable
          if (n_points != n_comp)
            mooseError(" Number of points ",
                       n_points,
                       " does not equal to number of variable components ",
                       n_comp);

          unsigned int offset = 0;
          for (auto & point : points)
          {
            dof_id_type from_dof = from_elem->dof_number(from_sys_num, from_var_num, offset++);
            src_pts.push_back(point);
            src_vals.push_back(from_solution(from_dof));
          }
        }
      }

      // We have only set local values - prepare for use by gathering remote gata
      idi->prepare_for_use();

      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
      {
        if (_multi_app->hasLocalApp(i))
        {
          Moose::ScopedCommSwapper swapper(_multi_app->comm());

          // Loop over the master nodes and set the value of the variable
          System * to_sys = find_sys(_multi_app->appProblemBase(i).es(), _to_var_name);

          unsigned int sys_num = to_sys->number();
          unsigned int var_num = to_sys->variable_number(_to_var_name);
          NumericVector<Real> & solution = _multi_app->appTransferVector(i, _to_var_name);

          MeshBase * mesh = NULL;

          if (_displaced_target_mesh && _multi_app->appProblemBase(i).getDisplacedProblem())
            mesh = &_multi_app->appProblemBase(i).getDisplacedProblem()->mesh().getMesh();
          else
            mesh = &_multi_app->appProblemBase(i).mesh().getMesh();

          auto & to_fe_type = to_sys->variable_type(var_num);
          bool to_is_constant = to_fe_type.order == CONSTANT;
          bool to_is_nodal = to_fe_type.family == LAGRANGE;

          if (to_fe_type.order > FIRST && !to_is_nodal)
            mooseError("We don't currently support second order or higher elemental variable ");

          if (to_is_nodal)
          {
            for (const auto & node : mesh->local_node_ptr_range())
            {
              Point actual_position = *node + _multi_app->position(i);

              if (node->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this node
              {
                std::vector<Point> pts;
                std::vector<Number> vals;

                pts.push_back(actual_position);
                vals.resize(1);

                idi->interpolate_field_data(vars, pts, vals);

                Real value = vals.front();

                // The zero only works for LAGRANGE!
                if (node->n_comp(from_sys_num, from_var_num) == 0)
                  continue;

                dof_id_type dof = node->dof_number(sys_num, var_num, 0);

                solution.set(dof, value);
              }
            }
          }
          else // Elemental
          {
            std::vector<Point> points;
            for (auto & elem : as_range(mesh->local_elements_begin(), mesh->local_elements_end()))
            {
              // Skip this element if the variable has no dofs at it.
              if (elem->n_dofs(sys_num, var_num) < 1)
                continue;

              points.clear();
              if (to_is_constant)
                points.push_back(elem->centroid());
              else
                for (auto & node : elem->node_ref_range())
                  points.push_back(node);

              auto n_points = points.size();
              unsigned int n_comp = elem->n_comp(sys_num, var_num);
              // We assume each point corresponds to one component of elemental variable
              if (n_points != n_comp)
                mooseError(" Number of points ",
                           n_points,
                           " does not equal to number of variable components ",
                           n_comp);

              unsigned int offset = 0;
              for (auto & point : points)
              {
                Point actual_position = point + _multi_app->position(i);

                std::vector<Point> pts;
                std::vector<Number> vals;

                pts.push_back(actual_position);
                vals.resize(1);

                idi->interpolate_field_data(vars, pts, vals);

                Real value = vals.front();

                dof_id_type dof = elem->dof_number(sys_num, var_num, offset++);

                solution.set(dof, value);
              } // point
            }   // auto elem
          }     // else

          solution.close();
          to_sys->update();
        }
      }

      delete idi;

      break;
    }
    case FROM_MULTIAPP:
    {
      FEProblemBase & to_problem = _multi_app->problemBase();
      MooseVariableFEBase & to_var = to_problem.getVariable(
          0, _to_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);
      SystemBase & to_system_base = to_var.sys();

      System & to_sys = to_system_base.system();

      NumericVector<Real> & to_solution = *to_sys.solution;

      unsigned int to_sys_num = to_sys.number();

      // Only works with a serialized mesh to transfer to!
      mooseAssert(to_sys.get_mesh().is_serial(),
                  "MultiAppInterpolationTransfer only works with ReplicatedMesh!");

      unsigned int to_var_num = to_sys.variable_number(to_var.name());

      // EquationSystems & to_es = to_sys.get_equation_systems();

      MeshBase * to_mesh = NULL;

      if (_displaced_target_mesh && to_problem.getDisplacedProblem())
        to_mesh = &to_problem.getDisplacedProblem()->mesh().getMesh();
      else
        to_mesh = &to_problem.mesh().getMesh();

      auto & fe_type = to_sys.variable_type(to_var_num);
      bool is_constant = fe_type.order == CONSTANT;
      bool is_nodal = fe_type.family == LAGRANGE;

      if (fe_type.order > FIRST && !is_nodal)
        mooseError("We don't currently support second order or higher elemental variable ");

      InverseDistanceInterpolation<LIBMESH_DIM> * idi;

      switch (_interp_type)
      {
        case 0:
          idi = new InverseDistanceInterpolation<LIBMESH_DIM>(to_sys.comm(), _num_points, _power);
          break;
        case 1:
          idi = new RadialBasisInterpolation<LIBMESH_DIM>(to_sys.comm(), _radius);
          break;
        default:
          mooseError("Unknown interpolation type!");
      }

      std::vector<Point> & src_pts(idi->get_source_points());
      std::vector<Number> & src_vals(idi->get_source_vals());

      std::vector<std::string> field_vars;
      field_vars.push_back(_to_var_name);
      idi->set_field_variables(field_vars);

      std::vector<std::string> vars;
      vars.push_back(_to_var_name);

      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
      {
        if (!_multi_app->hasLocalApp(i))
          continue;

        Moose::ScopedCommSwapper swapper(_multi_app->comm());

        FEProblemBase & from_problem = _multi_app->appProblemBase(i);
        MooseVariableFEBase & from_var =
            from_problem.getVariable(0,
                                     _from_var_name,
                                     Moose::VarKindType::VAR_ANY,
                                     Moose::VarFieldType::VAR_FIELD_STANDARD);
        SystemBase & from_system_base = from_var.sys();

        System & from_sys = from_system_base.system();
        unsigned int from_sys_num = from_sys.number();

        unsigned int from_var_num = from_sys.variable_number(from_var.name());

        auto & from_fe_type = from_sys.variable_type(from_var_num);
        bool from_is_constant = from_fe_type.order == CONSTANT;
        bool from_is_nodal = from_fe_type.family == LAGRANGE;

        if (from_fe_type.order > FIRST && !is_nodal)
          mooseError("We don't currently support second order or higher elemental variable ");
        // EquationSystems & from_es = from_sys.get_equation_systems();

        NumericVector<Number> & from_solution = *from_sys.solution;

        MeshBase * from_mesh = NULL;

        if (_displaced_source_mesh && from_problem.getDisplacedProblem())
          from_mesh = &from_problem.getDisplacedProblem()->mesh().getMesh();
        else
          from_mesh = &from_problem.mesh().getMesh();

        Point app_position = _multi_app->position(i);

        if (from_is_nodal)
        {
          for (const auto & from_node : from_mesh->local_node_ptr_range())
          {
            // Assuming LAGRANGE!
            if (from_node->n_comp(from_sys_num, from_var_num) == 0)
              continue;

            dof_id_type from_dof = from_node->dof_number(from_sys_num, from_var_num, 0);

            src_pts.push_back(*from_node + app_position);
            src_vals.push_back(from_solution(from_dof));
          }
        }
        else
        {
          std::vector<Point> points;
          for (auto & from_element :
               as_range(from_mesh->local_elements_begin(), from_mesh->local_elements_end()))
          {
            // Assuming LAGRANGE!
            if (from_element->n_comp(from_sys_num, from_var_num) == 0)
              continue;

            points.clear();
            // grap sample points
            // for constant shape function, we take the element centroid
            if (from_is_constant)
              points.push_back(from_element->centroid());
            // for higher order method, we take all nodes of element
            // this works for the first order L2 Lagrange.
            else
              for (auto & node : from_element->node_ref_range())
                points.push_back(node);

            auto n_points = points.size();
            unsigned int n_comp = from_element->n_comp(from_sys_num, from_var_num);
            unsigned int offset = 0;
            // We assume each point corresponds to one component of elemental variable
            if (n_points != n_comp)
              mooseError(" Number of points ",
                         n_points,
                         " does not equal to number of variable components ",
                         n_comp);

            for (auto & point : points)
            {
              dof_id_type from_dof = from_element->dof_number(from_sys_num, from_var_num, offset++);
              src_pts.push_back(point + app_position);
              src_vals.push_back(from_solution(from_dof));
            } // point
          }   // from_element
        }     // else
      }

      // We have only set local values - prepare for use by gathering remote gata
      idi->prepare_for_use();

      // Now do the interpolation to the target system
      if (is_nodal)
      {
        for (auto & node : as_range(to_mesh->local_nodes_begin(), to_mesh->local_nodes_end()))
        {
          if (node->n_dofs(to_sys_num, to_var_num) > 0) // If this variable has dofs at this node
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
        std::vector<Point> points;
        for (auto & elem : as_range(to_mesh->local_elements_begin(), to_mesh->local_elements_end()))
        {
          // Assuming LAGRANGE!
          if (elem->n_comp(to_sys_num, to_var_num) == 0)
            continue;

          points.clear();
          // grap sample points
          // for constant shape function, we take the element centroid
          if (is_constant)
            points.push_back(elem->centroid());
          // for higher order method, we take all nodes of element
          // this works for the first order L2 Lagrange.
          else
            for (auto & node : elem->node_ref_range())
              points.push_back(node);

          auto n_points = points.size();
          unsigned int n_comp = elem->n_comp(to_sys_num, to_var_num);
          // We assume each point corresponds to one component of elemental variable
          if (n_points != n_comp)
            mooseError(" Number of points ",
                       n_points,
                       " does not equal to number of variable components ",
                       n_comp);

          unsigned int offset = 0;
          for (auto & point : points)
          {
            std::vector<Point> pts;
            std::vector<Number> vals;

            pts.push_back(point);
            vals.resize(1);

            idi->interpolate_field_data(vars, pts, vals);

            Real value = vals.front();

            dof_id_type dof = elem->dof_number(to_sys_num, to_var_num, offset++);

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

  _console << "Finished InterpolationTransfer " << name() << std::endl;

  postExecute();
}

Node *
MultiAppInterpolationTransfer::getNearestNode(const Point & p,
                                              Real & distance,
                                              const MeshBase::const_node_iterator & nodes_begin,
                                              const MeshBase::const_node_iterator & nodes_end)
{
  distance = std::numeric_limits<Real>::max();
  Node * nearest = NULL;

  for (auto & node : as_range(nodes_begin, nodes_end))
  {
    Real current_distance = (p - *node).norm();

    if (current_distance < distance)
    {
      distance = current_distance;
      nearest = node;
    }
  }

  return nearest;
}
