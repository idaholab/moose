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
#include "MFEMVectorUtils.h"
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
  checkValidTransferProblemTypes<Moose::FEBackend::MFEM, Moose::FEBackend::LibMesh>();
}

MFEMProblem &
MultiApplibMeshToMFEMShapeEvaluationTransfer::getActiveToProblem()
{
  return static_cast<MFEMProblem &>(MFEMMultiAppTransfer::getActiveToProblem());
}

void
MultiApplibMeshToMFEMShapeEvaluationTransfer::transferVariables(bool is_target_local)
{
  for (const auto var_index : make_range(numToVar()))
  {
    // Declare map of processor ID to corresponding vector of libMesh points
    // on that processor to interpolate source libMesh variable at
    std::map<processor_id_type, std::vector<Point>> outgoing_points;
    mfem::Vector interp_vals;
    if (is_target_local)
    {
      // Generate list of points where the grid function will be evaluated
      mfem::ParGridFunction & to_gf =
          *getActiveToProblem().getGridFunction(getToVarName(var_index));
      mfem::ParFiniteElementSpace & to_pfespace = *to_gf.ParFESpace();
      if (to_gf.VectorDim() > 1)
        mooseError("MultiApplibMeshToMFEMShapeEvaluationTransfer does not support transfers of "
                   "vector variables from libMesh to MFEM-based subapps");
      mfem::Vector vxyz;
      mfem::Ordering::Type point_ordering;
      _mfem_projector.extractNodePositions(to_pfespace, vxyz, point_ordering);

      // Populate outgoing point locations map between processor and points vector for libMesh to
      // use in interpolation
      const int dim = to_pfespace.GetParMesh()->Dimension();
      const int nnodes = vxyz.Size() / dim;
      for (const auto i : make_range(nnodes))
      {
        libMesh::Point point_in_target_frame;
        for (const auto d : make_range(dim))
          point_in_target_frame(d) = vxyz[i + d * nnodes];

        const auto point_in_source_frame = mapPointToActiveSourceFrame(point_in_target_frame);
        for (const auto i_proc : make_range(n_processors()))
          outgoing_points[i_proc].push_back(point_in_source_frame);
      }
      interp_vals.SetSize(nnodes);
    }

    // Perform interpolation of libMesh variable at specified points
    interpolatelibMeshVariable(outgoing_points, var_index, interp_vals);

    // Project DoFs to MFEM GridFunction
    if (is_target_local)
    {
      mfem::ParGridFunction & to_gf =
          *getActiveToProblem().getGridFunction(getToVarName(var_index));
      mfem::Ordering::Type libmesh_interp_ordering(mfem::Ordering::Type::byNODES);
      _mfem_projector.projectNodalValues(interp_vals, libmesh_interp_ordering, to_gf);
    }
  }
}

void
MultiApplibMeshToMFEMShapeEvaluationTransfer::interpolatelibMeshVariable(
    std::map<processor_id_type, std::vector<Point>> & outgoing_points,
    const unsigned int var_index,
    mfem::Vector & interp_vals)
{
  FEProblemBase & from_problem = getActiveFromProblem();
  MooseVariableFieldBase & from_var = from_problem.getVariable(0,
                                                               getFromVarName(var_index),
                                                               Moose::VarKindType::VAR_ANY,
                                                               Moose::VarFieldType::VAR_FIELD_ANY);
  System & from_sys = from_var.sys().system();
  unsigned int from_var_num = from_sys.variable_number(getFromVarName(var_index));
  // Construct a local mesh function for each origin problem
  libMesh::MeshFunction local_meshfuns(
      getlibMeshEquationSystem(from_problem, _displaced_source_mesh),
      *from_sys.current_local_solution,
      from_sys.get_dof_map(),
      from_var_num);
  local_meshfuns.init();
  local_meshfuns.enable_out_of_mesh_mode(getMFEMOutOfMeshValue());

  // Evaluate interpolated values at incoming points.
  auto gather_functor =
      [this, &local_meshfuns](processor_id_type /*pid*/,
                              const std::vector<Point> & incoming_points,
                              std::vector<mfem::real_t> & vals_for_incoming_points)
  {
    vals_for_incoming_points.resize(incoming_points.size(), getMFEMOutOfMeshValue());
    // Compute interpolation values of the libMesh variable at all requested points
    for (const auto i_pt : index_range(incoming_points))
      vals_for_incoming_points[i_pt] = local_meshfuns(incoming_points[i_pt]);
  };
  // Copy data out to interp_vals
  auto action_functor =
      [&interp_vals, this](processor_id_type /*pid*/,
                           const std::vector<Point> & /*my_outgoing_points*/,
                           const std::vector<mfem::real_t> & vals_for_outgoing_points)
  {
    for (const auto i : make_range(interp_vals.Size()))
    {
      const auto val = vals_for_outgoing_points[i];
      if (val == getMFEMOutOfMeshValue())
        continue;
      interp_vals(i) = val;
    }
  };

  // Set interpolated field values at points on local processor
  interp_vals = getMFEMOutOfMeshValue(); // default to the out-of-mesh value
  // We assume incoming_vals is ordered in the same way as outgoing_points
  libMesh::Parallel::pull_parallel_vector_data(
      comm(), outgoing_points, gather_functor, action_functor, (mfem::real_t *)(nullptr));
}

#endif
