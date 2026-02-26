//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MultiApplibMeshToMFEMShapeEvaluationTransfer.h"
#include "FEProblemBase.h"
#include "MultiApp.h"
#include "SystemBase.h"
#include "MFEMProblem.h"
#include "MFEMMesh.h"
#include "MFEMVectorFromlibMeshPoint.h"

#include "libmesh/mesh_function.h"
#include "libmesh/parallel_algebra.h" // for communicator send and receive stuff

registerMooseObject("MooseApp", MultiApplibMeshToMFEMShapeEvaluationTransfer);

InputParameters
MultiApplibMeshToMFEMShapeEvaluationTransfer::validParams()
{
  InputParameters params = MFEMMultiAppTransfer::validParams();
  params.addClassDescription("Transfers variable values from a libMesh based application to an "
                             "MFEM application, using shape function evaluations.");
  return params;
}

MultiApplibMeshToMFEMShapeEvaluationTransfer::MultiApplibMeshToMFEMShapeEvaluationTransfer(
    InputParameters const & params)
  : MFEMMultiAppTransfer(params)
{
  checkValidTransferProblemTypes<MFEMProblem, FEProblemBase>();
}

void
MultiApplibMeshToMFEMShapeEvaluationTransfer::transferVariables()
{
  for (unsigned v = 0; v < numToVar(); ++v)
    setMFEMGridFunctionValuesFromlibMesh(v, getActiveToProblem());
}

void
MultiApplibMeshToMFEMShapeEvaluationTransfer::setMFEMGridFunctionValuesFromlibMesh(
    const unsigned int var_index, MFEMProblem & to_problem)
{
  // Generate list of points where the grid function will be evaluated
  mfem::ParGridFunction & to_gf =
      *to_problem.getProblemData().gridfunctions.Get(getToVarName(var_index));
  mfem::ParFiniteElementSpace & to_pfespace = *to_gf.ParFESpace();
  mfem::Vector vxyz;
  mfem::Ordering::Type point_ordering;
  _mfem_projector.extractNodePositions(to_pfespace, vxyz, point_ordering);

  // Point locations needed to send to from-domain
  // processor to points
  const int NE = to_pfespace.GetParMesh()->GetNE();
  const int nsp = to_pfespace.GetTypicalFE()->GetNodes().GetNPoints();
  const int dim = to_pfespace.GetParMesh()->Dimension();
  const int nodes_cnt = vxyz.Size() / dim;
  const int to_gf_ncomp = to_gf.VectorDim();
  std::map<processor_id_type, std::vector<Point>> outgoing_points;

  for (int i = 0; i < nodes_cnt * to_gf_ncomp; i++)
  {
    for (processor_id_type i_proc = 0; i_proc < n_processors(); ++i_proc)
    {
      if (dim == 3)
      {
        const mfem::Vector transformed_node({vxyz[i], vxyz[i + NE * nsp], vxyz[i + 2 * NE * nsp]});
        outgoing_points[i_proc].push_back(
            Moose::MFEM::libMeshPointFromMFEMVector(transformed_node));
      }
      else
      {
        const mfem::Vector transformed_node({vxyz[i], vxyz[i + NE * nsp]});
        outgoing_points[i_proc].push_back(
            Moose::MFEM::libMeshPointFromMFEMVector(transformed_node));
      }
    }
  }
  // Evaluate source grid function at target points
  mfem::Vector interp_vals(nodes_cnt * to_gf_ncomp);
  std::vector<libMesh::MeshFunction> local_meshfuns;
  local_meshfuns.clear();
  local_meshfuns.reserve(_from_problems.size());
  // Construct a local mesh function for each origin problem
  for (unsigned int i_from = 0; i_from < _from_problems.size(); ++i_from)
  {
    FEProblemBase & from_problem = *_from_problems[i_from];
    MooseVariableFieldBase & from_var =
        from_problem.getVariable(0,
                                 _from_var_names[var_index],
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_ANY);
    System & from_sys = from_var.sys().system();
    unsigned int from_var_num = from_sys.variable_number(getFromVarName(var_index));
    local_meshfuns.emplace_back(
        from_problem.es(), *from_sys.current_local_solution, from_sys.get_dof_map(), from_var_num);
    local_meshfuns.back().init();
    local_meshfuns.back().enable_out_of_mesh_mode(std::numeric_limits<Real>::infinity());
  }

  /**
   * Gather all of the evaluations, pick out the best ones for each point, and
   * apply them to the solution vector.  When we are transferring from
   * multiapps, there may be multiple overlapping apps for a particular point.
   * In that case, we'll try to use the value from the app with the lowest id.
   */

  // Fill values and app ids for incoming points
  // We are responsible to compute values for these incoming points
  auto gather_functor =
      [this,
       &local_meshfuns](processor_id_type /*pid*/,
                        const std::vector<Point> & incoming_points,
                        std::vector<std::pair<Real, unsigned int>> & vals_ids_for_incoming_points)
  {
    vals_ids_for_incoming_points.resize(incoming_points.size(), std::make_pair(OutOfMeshValue, 0));
    for (MooseIndex(incoming_points.size()) i_pt = 0; i_pt < incoming_points.size(); ++i_pt)
    {
      Point pt = incoming_points[i_pt];

      // Loop until we've found the lowest-ranked app that actually contains
      // the quadrature point.
      for (MooseIndex(_from_problems.size()) i_from = 0;
           i_from < _from_problems.size() &&
           vals_ids_for_incoming_points[i_pt].first == OutOfMeshValue;
           ++i_from)
      {
        const auto from_global_num =
            _current_direction == TO_MULTIAPP ? 0 : _from_local2global_map[i_from];
        // Use mesh function to compute interpolation values
        vals_ids_for_incoming_points[i_pt].first =
            (local_meshfuns[i_from])(_from_transforms[from_global_num]->mapBack(pt));
        // Record problem ID as well
        switch (_current_direction)
        {
          case FROM_MULTIAPP:
            vals_ids_for_incoming_points[i_pt].second = _from_local2global_map[i_from];
            break;
          case TO_MULTIAPP:
            vals_ids_for_incoming_points[i_pt].second = _to_local2global_map[i_from];
            break;
          default:
            mooseError("Unsupported direction");
        }
      }
    }
  };

  // Incoming values and APP ids for outgoing points
  std::map<processor_id_type, std::vector<std::pair<Real, unsigned int>>> incoming_vals_ids;
  // Copy data out to incoming_vals_ids
  auto action_functor =
      [&incoming_vals_ids](
          processor_id_type pid,
          const std::vector<Point> & /*my_outgoing_points*/,
          const std::vector<std::pair<Real, unsigned int>> & vals_ids_for_outgoing_points)
  {
    // This lambda function might be called multiple times
    incoming_vals_ids[pid].reserve(vals_ids_for_outgoing_points.size());
    // Copy data for processor 'pid'
    std::copy(vals_ids_for_outgoing_points.begin(),
              vals_ids_for_outgoing_points.end(),
              std::back_inserter(incoming_vals_ids[pid]));
  };

  // We assume incoming_vals_ids is ordered in the same way as outgoing_points
  // Hopefully, pull_parallel_vector_data will not mess up this
  const std::pair<Real, unsigned int> * ex = nullptr;
  libMesh::Parallel::pull_parallel_vector_data(
      comm(), outgoing_points, gather_functor, action_functor, ex);

  for (int i = 0; i < interp_vals.Size(); i++)
  {
    for (auto & group : incoming_vals_ids)
    {
      double val = group.second[i].first;
      if (val == std::numeric_limits<Real>::infinity())
        continue;
      interp_vals[i] = val;
    }
  }

  // Project DoFs to MFEM GridFunction
  mfem::Ordering::Type libmesh_interp_ordering(mfem::Ordering::Type::byNODES);
  _mfem_projector.projectNodalValues(interp_vals, libmesh_interp_ordering, to_gf);
}

#endif
