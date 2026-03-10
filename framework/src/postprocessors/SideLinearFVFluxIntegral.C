//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideLinearFVFluxIntegral.h"
#include "LinearFVBoundaryCondition.h"
#include "LinearFVFluxKernel.h"
#include <set>

registerMooseObject("MooseApp", SideLinearFVFluxIntegral);

InputParameters
SideLinearFVFluxIntegral::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredParam<std::vector<std::string>>(
      "linearfvkernels", "List of LinearFVFluxKernels whose boundary fluxes are integrated.");
  params.addClassDescription(
      "Computes the side integral of selected LinearFVFluxKernel boundary flux contributions.");
  return params;
}

SideLinearFVFluxIntegral::SideLinearFVFluxIntegral(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _kernel_names(getParam<std::vector<std::string>>("linearfvkernels")),
    _variable_number(0),
    _system_number(0)
{
  _qp_integration = false;
}

void
SideLinearFVFluxIntegral::initialSetup()
{
  SideIntegralPostprocessor::initialSetup();

  // Kernels are constructed after postprocessors, so we fetch them here.
  auto base_query = _fe_problem.theWarehouse()
                        .query()
                        .condition<AttribSystem>("LinearFVFluxKernel")
                        .condition<AttribThread>(_tid);

  _kernel_objects.clear();
  for (const auto & name : _kernel_names)
  {
    std::vector<LinearFVFluxKernel *> kernels;
    auto query = base_query;
    query.condition<AttribName>(name).queryInto(kernels);
    if (kernels.empty())
      paramError("linearfvkernels",
                 "The given LinearFVFluxKernel with name '",
                 name,
                 "' was not found! This can be due to the kernel not existing in the "
                 "'LinearFVKernels' block or the kernel not inheriting from LinearFVFluxKernel.");

    _kernel_objects.push_back(kernels[0]);
  }

  if (_kernel_objects.empty())
    paramError("linearfvkernels", "At least one kernel must be provided.");

  // Cache shared variable metadata and verify all kernels act on that same variable.
  const auto & first_variable = _kernel_objects.front()->variable();
  _variable_name = first_variable.name();
  _variable_number = first_variable.number();
  _system_number = first_variable.sys().number();

  for (const auto kernel_ptr : _kernel_objects)
    if (kernel_ptr->variable().name() != _variable_name)
      paramError("linearfvkernels",
                 "All kernels in 'linearfvkernels' must act on the same variable name. The "
                 "kernel '",
                 kernel_ptr->name(),
                 "' acts on variable '",
                 kernel_ptr->variable().name(),
                 "', while the first kernel acts on variable '",
                 _variable_name,
                 "'.");

  // Postprocessors run initialSetup before variable-side BC caches are guaranteed ready,
  // so we resolve BCs directly from the warehouse.
  auto bc_query = _fe_problem.theWarehouse()
                      .query()
                      .condition<AttribSystem>("LinearFVBoundaryCondition")
                      .condition<AttribThread>(_tid)
                      .condition<AttribVar>(_variable_number)
                      .condition<AttribSysNum>(_system_number);

  _boundary_bcs.clear();
  for (const auto boundary_id : boundaryIDs())
  {
    std::vector<LinearFVBoundaryCondition *> bcs;
    auto bc_query_copy = bc_query;
    bc_query_copy.condition<AttribBoundaries>(std::set<BoundaryID>({boundary_id})).queryInto(bcs);

    if (bcs.empty())
      paramError("linearfvkernels",
                 "Variable '",
                 _variable_name,
                 "' does not have a LinearFVBoundaryCondition on boundary '",
                 _mesh.getBoundaryName(boundary_id),
                 "'.");

    if (bcs.size() > 1)
      paramError("linearfvkernels",
                 "Variable '",
                 _variable_name,
                 "' has multiple LinearFVBoundaryCondition objects on boundary '",
                 _mesh.getBoundaryName(boundary_id),
                 "'.");

    _boundary_bcs.emplace(boundary_id, bcs[0]);
  }
}

Real
SideLinearFVFluxIntegral::computeFaceInfoIntegral(const FaceInfo * const fi)
{
  mooseAssert(fi, "FaceInfo should not be null.");
  mooseAssert(fi->boundaryIDs().size() == 1, "Expected exactly one boundary per face.");

  const auto boundary_id = *fi->boundaryIDs().begin();
  const auto face_type = fi->faceType(std::make_pair(_variable_number, _system_number));
  if (face_type != FaceInfo::VarFaceNeighbors::ELEM &&
      face_type != FaceInfo::VarFaceNeighbors::NEIGHBOR)
    mooseError("Cannot compute boundary flux on boundary '",
               _mesh.getBoundaryName(boundary_id),
               "' because the variable face type is not boundary-only.");

  const auto bc_it = _boundary_bcs.find(boundary_id);
  auto * bc = bc_it->second;
  bc->setupFaceData(fi, face_type);

  Real flux_value = 0.0;
  for (const auto kernel_ptr : _kernel_objects)
  {
    kernel_ptr->setupFaceData(fi);
    // SideIntegralPostprocessor multiplies by geometric factors, so we just do 1 here.
    kernel_ptr->setCurrentFaceArea(1.0);
    flux_value += kernel_ptr->computeBoundaryFlux(*bc);
  }

  return flux_value;
}

Real
SideLinearFVFluxIntegral::computeQpIntegral()
{
  mooseError("We should never call this function.");
}
