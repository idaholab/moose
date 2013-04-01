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

#include "MultiAppPostprocessorInterpolationTransfer.h"

// Moose
#include "MooseTypes.h"
#include "FEProblem.h"

// libMesh
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"
#include "libmesh/radial_basis_interpolation.h"

template<>
InputParameters validParams<MultiAppPostprocessorInterpolationTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<AuxVariableName>("variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<PostprocessorName>("postprocessor", "The Postprocessor to interpolate.");
  params.addParam<unsigned int>("num_points", 3, "The number of nearest points to use for interpolation.");
  params.addParam<Real>("power", 2, "The polynomial power to use for calculation of the decay in the interpolation.");

  MooseEnum interp_type("inverse_distance, radial_basis", "inverse_distance");

  params.addParam<MooseEnum>("interp_type", interp_type, "The algorithm to use for interpolation.");

  params.addParam<Real>("radius", -1, "Radius to use for radial_basis interpolation.  If negative then the radius is taken as the max distance between points.");

  return params;
}

MultiAppPostprocessorInterpolationTransfer::MultiAppPostprocessorInterpolationTransfer(const std::string & name, InputParameters parameters) :
    MultiAppTransfer(name, parameters),
    _postprocessor(getParam<PostprocessorName>("postprocessor")),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _num_points(getParam<unsigned int>("num_points")),
    _power(getParam<Real>("power")),
    _interp_type(getParam<MooseEnum>("interp_type")),
    _radius(getParam<Real>("radius"))
{
}

void
MultiAppPostprocessorInterpolationTransfer::execute()
{
  switch(_direction)
  {
    case TO_MULTIAPP:
    {
      mooseError("Can't interpolate to a MultiApp!!");
      break;
    }
    case FROM_MULTIAPP:
    {
      InverseDistanceInterpolation<LIBMESH_DIM> * idi;

      switch(_interp_type)
      {
        case 0:
          idi = new InverseDistanceInterpolation<LIBMESH_DIM>(_num_points, _power);
          break;
        case 1:
          idi = new RadialBasisInterpolation<LIBMESH_DIM>(_radius);
          break;
        default:
          mooseError("Unknown interpolation type!");
      }

      std::vector<Point>  &src_pts  (idi->get_source_points());
      std::vector<Number> &src_vals (idi->get_source_vals());

      std::vector<std::string> field_vars;
      field_vars.push_back(_to_var_name);
      idi->set_field_variables(field_vars);

      {
        for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
        {
          if(_multi_app->hasLocalApp(i) && _multi_app->isRootProcessor())
          {
            src_pts.push_back(_multi_app->position(i));
            src_vals.push_back(_multi_app->appPostprocessorValue(i,_postprocessor));
          }
        }
      }

      // We have only set local values - prepare for use by gathering remote gata
      idi->prepare_for_use();


      // Loop over the master nodes and set the value of the variable
      {
        System * to_sys = find_sys(_multi_app->problem()->es(), _to_var_name);

        unsigned int sys_num = to_sys->number();
        unsigned int var_num = to_sys->variable_number(_to_var_name);

        NumericVector<Real> & solution = *to_sys->solution;

        MooseMesh & mesh = _multi_app->problem()->mesh();

        std::vector<std::string> vars;

        vars.push_back(_to_var_name);

        MeshBase::const_node_iterator node_it = mesh.localNodesBegin();
        MeshBase::const_node_iterator node_end = mesh.localNodesEnd();

        for(; node_it != node_end; ++node_it)
        {
          Node * node = *node_it;

          if(node->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this node
          {
            std::vector<Point> pts;
            std::vector<Number> vals;

            pts.push_back(*node);
            vals.resize(1);

            idi->interpolate_field_data(vars, pts, vals);

            Real value = vals.front();

            // The zero only works for LAGRANGE!
            unsigned int dof = node->dof_number(sys_num, var_num, 0);

            solution.set(dof, value);
          }
        }

        solution.close();
      }

      _multi_app->problem()->es().update();

      delete idi;

      break;
    }
  }
}
