//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppPostprocessorInterpolationTransfer.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MultiApp.h"
#include "MooseAppCoordTransform.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/system.h"
#include "libmesh/radial_basis_interpolation.h"

using namespace libMesh;

registerMooseObject("MooseApp", MultiAppPostprocessorInterpolationTransfer);

InputParameters
MultiAppPostprocessorInterpolationTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addClassDescription("Transfer postprocessor data from sub-application into field data on "
                             "the parent application.");
  params.addRequiredParam<AuxVariableName>(
      "variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<PostprocessorName>("postprocessor", "The Postprocessor to interpolate.");
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

MultiAppPostprocessorInterpolationTransfer::MultiAppPostprocessorInterpolationTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _postprocessor(getParam<PostprocessorName>("postprocessor")),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _num_points(getParam<unsigned int>("num_points")),
    _power(getParam<Real>("power")),
    _interp_type(getParam<MooseEnum>("interp_type")),
    _radius(getParam<Real>("radius")),
    _nodal(false)
{
  if (isParamValid("to_multi_app"))
    paramError("to_multi_app", "Unused parameter; only from-MultiApp transfers are implemented");

  auto & to_fe_type =
      getFromMultiApp()->problemBase().getStandardVariable(0, _to_var_name).feType();
  if ((to_fe_type.order != CONSTANT || to_fe_type.family != MONOMIAL) &&
      (to_fe_type.order != FIRST || to_fe_type.family != LAGRANGE))
    paramError("variable", "Must be either CONSTANT MONOMIAL or FIRST LAGRANGE");

  _nodal = to_fe_type.family == LAGRANGE;
}

void
MultiAppPostprocessorInterpolationTransfer::execute()
{
  TIME_SECTION("MultiAppPostprocessorInterpolationTransfer::execute()",
               5,
               "Transferring/interpolating postprocessors");

  switch (_current_direction)
  {
    case TO_MULTIAPP:
    {
      mooseError("Interpolation from a variable to a MultiApp postprocessors has not been "
                 "implemented. Use MultiAppVariableValueSamplePostprocessorTransfer!");
      break;
    }
    case FROM_MULTIAPP:
    {
      std::unique_ptr<InverseDistanceInterpolation<LIBMESH_DIM>> idi;

      switch (_interp_type)
      {
        case 0:
          idi = std::make_unique<InverseDistanceInterpolation<LIBMESH_DIM>>(
              _communicator, _num_points, _power);
          break;
        case 1:
          idi = std::make_unique<RadialBasisInterpolation<LIBMESH_DIM>>(_communicator, _radius);
          break;
        default:
          mooseError("Unknown interpolation type!");
      }

      std::vector<Point> & src_pts(idi->get_source_points());
      std::vector<Number> & src_vals(idi->get_source_vals());

      std::vector<std::string> field_vars;
      field_vars.push_back(_to_var_name);
      idi->set_field_variables(field_vars);

      {
        mooseAssert(_to_transforms.size() == 1, "There should only be one transform here");
        const auto & to_coord_transform = *_to_transforms[0];
        for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
        {
          if (getFromMultiApp()->hasLocalApp(i) && getFromMultiApp()->isRootProcessor())
          {
            // Evaluation of the _from_transform at the origin yields the transformed position of
            // the from multi-app
            if (!getFromMultiApp()->runningInPosition())
              src_pts.push_back(to_coord_transform.mapBack((*_from_transforms[i])(Point(0))));
            else
              // if running in position, the subapp mesh has been transformed so the translation
              // is no longer applied by the transform
              src_pts.push_back(to_coord_transform.mapBack(
                  (*_from_transforms[i])(getFromMultiApp()->position(i))));
            src_vals.push_back(getFromMultiApp()->appPostprocessorValue(i, _postprocessor));
          }
        }
      }

      // We have only set local values - prepare for use by gathering remote data
      idi->prepare_for_use();

      // Loop over the parent app nodes and set the value of the variable
      {
        System * to_sys = find_sys(getFromMultiApp()->problemBase().es(), _to_var_name);

        unsigned int sys_num = to_sys->number();
        unsigned int var_num = to_sys->variable_number(_to_var_name);

        NumericVector<Real> & solution = *to_sys->solution;

        MooseMesh & mesh = getFromMultiApp()->problemBase().mesh();

        std::vector<std::string> vars;

        vars.push_back(_to_var_name);

        if (_nodal)
        {
          // handle linear lagrange shape functions
          for (const auto & node : as_range(mesh.localNodesBegin(), mesh.localNodesEnd()))
          {
            if (node->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this node
            {
              std::vector<Point> pts;
              std::vector<Number> vals;

              pts.push_back(*node);
              vals.resize(1);

              idi->interpolate_field_data(vars, pts, vals);

              Real value = vals.front();

              // The zero only works for LAGRANGE!
              dof_id_type dof = node->dof_number(sys_num, var_num, 0);

              solution.set(dof, value);
            }
          }
        }
        else
        {
          // handle constant monomial shape functions
          for (const auto & elem :
               as_range(mesh.getMesh().local_elements_begin(), mesh.getMesh().local_elements_end()))
          {
            // Exclude the elements without dofs
            if (elem->n_dofs(sys_num, var_num) > 0)
            {
              std::vector<Point> pts;
              std::vector<Number> vals;

              pts.push_back(elem->vertex_average());
              vals.resize(1);

              idi->interpolate_field_data(vars, pts, vals);

              Real value = vals.front();

              dof_id_type dof = elem->dof_number(sys_num, var_num, 0);
              solution.set(dof, value);
            }
          }
        }

        solution.close();
      }

      getFromMultiApp()->problemBase().es().update();

      break;
    }
  }
}
