//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMProblem.h"
#include "MFEMInitialCondition.h"
#include "MFEMVariable.h"
#include "MFEMIndicator.h"
#include "MFEMSubMesh.h"
#include "MFEMFunctorMaterial.h"
#include "MFEMSubMeshTransfer.h"
#include "MFEMExecutedObject.h"
#include "MFEMFESpaceHierarchy.h"
#include "Postprocessor.h"
#include "VectorPostprocessor.h"
#include "MFEMNonlinearSolverBase.h"

#include "libmesh/string_to_enum.h"

#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <deque>
#include <sstream>

registerMooseMFEMObject("MooseApp", Problem);

namespace Moose::MFEM
{
InputParameters
Problem::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addClassDescription("Problem type for building and solving the finite element problem "
                             "using the MFEM finite element library.");
  MooseEnum numeric_types("real complex", "real");
  params.addParam<MooseEnum>("numeric_type", numeric_types, "Number type used for the problem");

  return params;
}

Problem::Problem(const InputParameters & params)
  : ExternalProblem(params), num_type{static_cast<int>(getParam<MooseEnum>("numeric_type"))}
{
  // Initialise Hypre for all MFEM problems.
  mfem::Hypre::Init();
  // Disable multithreading for all MFEM problems (including any libMesh or MFEM subapps).
  libMesh::libMeshPrivateData::_n_threads = 1;
  setMesh();
}

void
Problem::initialSetup()
{
  FEProblemBase::initialSetup();

  // MFEM indicators create their estimators during addIndicator(); markers still need an explicit
  // setup pass because they are no longer initialized through the libMesh/MOOSE user-object path.
  std::vector<RefinementMarker *> markers;
  theWarehouse().query().condition<AttribSystem>("Marker").queryInto(markers);
  for (auto marker : markers)
    marker->initialSetup();
}

void
Problem::execute(const ExecFlagType & exec_type)
{
  setCurrentExecuteOnFlag(exec_type);
  executeMFEMObjects(exec_type);

  FEProblemBase::execute(exec_type);
}

void
Problem::setMesh()
{
  auto pmesh = mesh().getMFEMParMeshPtr();
  getProblemData().pmesh = pmesh;
  getProblemData().comm = pmesh->GetComm();
  getProblemData().num_procs = pmesh->GetNRanks();
  getProblemData().myid = pmesh->GetMyRank();
}

void
Problem::addIndicator(const std::string & user_object_name,
                      const std::string & name,
                      InputParameters & parameters)
{
  auto estimator = addObject<Indicator>(user_object_name, name, parameters).front();

  // construct the estimator itself
  estimator->createEstimator();
}

void
Problem::addMarker(const std::string & user_object_name,
                   const std::string & name,
                   InputParameters & parameters)
{
  getProblemData().refiner =
      addObject<RefinementMarker>(user_object_name, name, parameters).front();
}

void
Problem::addMFEMSolver(const std::string & user_object_name,
                       const std::string & name,
                       InputParameters & parameters)
{
  auto object = addObject<SolverBase>(user_object_name, name, parameters).front();

  if (auto lin_solver = std::dynamic_pointer_cast<LinearSolverBase>(object))
    getProblemData().jacobian_solver = lin_solver;
  else if (auto nonlinear_solver = std::dynamic_pointer_cast<NonlinearSolverBase>(object);
           nonlinear_solver)
    getProblemData().nonlinear_solver = nonlinear_solver;
  else
    mooseError(
        "Unsupported MFEM solver object type '", user_object_name, "' for solver '", name, "'.");
}

void
Problem::addBoundaryCondition(const std::string & bc_name,
                              const std::string & name,
                              InputParameters & parameters)
{
  auto bc = addObject<BoundaryCondition>(bc_name, name, parameters).front();
  const auto & mfem_bc = *bc;

  if (dynamic_cast<const IntegratedBC *>(&mfem_bc))
  {
    auto integrated_bc = std::dynamic_pointer_cast<IntegratedBC>(bc);
    auto eqsys = std::dynamic_pointer_cast<EquationSystem>(getProblemData().eqn_system);
    if (eqsys)
      eqsys->AddIntegratedBC(std::move(integrated_bc));
    else
      mooseError("Cannot add integrated BC with name '" + name +
                 "' because there is no corresponding equation system.");
  }
  else if (dynamic_cast<const ComplexIntegratedBC *>(&mfem_bc))
  {
    auto integrated_bc = std::dynamic_pointer_cast<ComplexIntegratedBC>(bc);
    auto eqsys = std::dynamic_pointer_cast<ComplexEquationSystem>(getProblemData().eqn_system);
    if (eqsys)
      eqsys->AddComplexIntegratedBC(std::move(integrated_bc));
    else
      mooseError("Cannot add complex integrated BC with name '" + name +
                 "' because there is no corresponding equation system.");
  }
  else if (dynamic_cast<const ComplexEssentialBC *>(&mfem_bc))
  {
    auto essential_bc = std::dynamic_pointer_cast<ComplexEssentialBC>(bc);
    auto eqsys = std::dynamic_pointer_cast<ComplexEquationSystem>(getProblemData().eqn_system);
    if (eqsys)
      eqsys->AddComplexEssentialBCs(std::move(essential_bc));
    else
      mooseError("Cannot add boundary condition with name '" + name +
                 "' because there is no corresponding equation system.");
  }
  else if (dynamic_cast<const EssentialBC *>(&mfem_bc))
  {
    auto essential_bc = std::dynamic_pointer_cast<EssentialBC>(bc);
    auto eqsys = std::dynamic_pointer_cast<EquationSystem>(getProblemData().eqn_system);
    if (eqsys)
      eqsys->AddEssentialBC(std::move(essential_bc));
    else
      mooseError("Cannot add boundary condition with name '" + name +
                 "' because there is no corresponding equation system.");
  }
  else
  {
    mooseError("Unsupported bc of type '", bc_name, "' and name '", name, "' detected.");
  }
}

void
Problem::addMaterial(const std::string &, const std::string &, InputParameters &)
{
  mooseError(
      "MFEM materials must be added through the 'FunctorMaterials' block and not 'Materials'");
}

void
Problem::addFunctorMaterial(const std::string & material_name,
                            const std::string & name,
                            InputParameters & parameters)
{
  addObject<FunctorMaterial>(material_name, name, parameters);
}

void
Problem::addFESpace(const std::string & type,
                    const std::string & name,
                    InputParameters & parameters)
{
  if (getProblemData().fespace_hierarchies.Has(name))
    mooseError("Cannot add FESpace '",
               name,
               "': a FESpaceHierarchy with the same name already exists. "
               "FESpaces and FESpaceHierarchies share the fespaces namespace.");

  auto & mfem_fespace = *addObject<FESpace>(type, name, parameters).front();

  // Register fespace and associated fe collection.
  getProblemData().fecs.Register(name, mfem_fespace.getFEC());
  getProblemData().fespaces.Register(name, mfem_fespace.getFESpace());
}

void
Problem::addFESpaceHierarchy(const std::string & type,
                             const std::string & name,
                             InputParameters & parameters)
{
  if (getProblemData().fespaces.Has(name))
    mooseError("Cannot add FESpaceHierarchy '",
               name,
               "': a FESpace with the same name already exists. "
               "FESpaces and FESpaceHierarchies share the fespaces namespace.");

  auto hierarchy_obj = addObject<FESpaceHierarchy>(type, name, parameters).front();
  auto hierarchy_shared = hierarchy_obj->getHierarchyShared();
  // Register the hierarchy for co-ownership by solvers.
  getProblemData().fespace_hierarchies.Register(name, hierarchy_shared);
  // Register the finest-level FESpace in fespaces under the hierarchy name so that
  // variables can say `fespace = <hierarchy_name>` without a separate FESpace definition.
  // The aliasing shared_ptr keeps the hierarchy alive as long as this entry lives.
  auto finest = std::shared_ptr<mfem::ParFiniteElementSpace>(
      hierarchy_shared,
      &static_cast<mfem::ParFiniteElementSpace &>(
          hierarchy_obj->getHierarchy().GetFinestFESpace()));
  getProblemData().fespaces.Register(name, finest);
}

void
Problem::addVariable(const std::string & var_type,
                     const std::string & var_name,
                     InputParameters & parameters)
{
  addGridFunction(var_type, var_name, parameters);
  // MOOSE variables store DoFs for the trial variable and its time derivatives up to second order;
  // MFEM GridFunctions store data for only one set of DoFs each, so we must add additional
  // GridFunctions for time derivatives.
  if (isTransient())
  {
    const auto time_derivative_var_name =
        getMFEMObject<Variable>("MooseVariableBase", var_name).getTimeDerivativeName();
    getProblemData().time_derivative_map.addTimeDerivativeAssociation(var_name,
                                                                      time_derivative_var_name);
    addGridFunction(var_type, time_derivative_var_name, parameters);
  }
}

void
Problem::addGridFunction(const std::string & var_type,
                         const std::string & var_name,
                         InputParameters & parameters)
{

  if (var_type == "MFEMVariable" || var_type == "MFEMComplexVariable")
  {
    // Add MFEM variable directly.
    if (var_type == "MFEMComplexVariable")
      addObject<ComplexVariable>(var_type, var_name, parameters);
    else
      addObject<Variable>(var_type, var_name, parameters);
  }
  else
  {
    // Add MOOSE variable.
    FEProblemBase::addVariable(var_type, var_name, parameters);

    // Add MFEM variable indirectly ("gridfunction").
    InputParameters mfem_variable_params = addMFEMFESpaceFromMOOSEVariable(parameters);
    addObject<Variable>("MFEMVariable", var_name, mfem_variable_params);
  }

  // Register gridfunction.
  if (var_type == "MFEMComplexVariable")
  {
    ComplexVariable & mfem_variable = getMFEMObject<ComplexVariable>("MooseVariableBase", var_name);
    getProblemData().cmplx_gridfunctions.Register(var_name, mfem_variable.getComplexGridFunction());
    if (mfem_variable.isScalar())
    {
      getCoefficients().declareScalar<mfem::GridFunctionCoefficient>(
          var_name + "_real", &mfem_variable.getComplexGridFunction()->real());
      getCoefficients().declareScalar<mfem::GridFunctionCoefficient>(
          var_name + "_imag", &mfem_variable.getComplexGridFunction()->imag());
    }
    else
    {
      getCoefficients().declareVector<mfem::VectorGridFunctionCoefficient>(
          var_name + "_real", &mfem_variable.getComplexGridFunction()->real());
      getCoefficients().declareVector<mfem::VectorGridFunctionCoefficient>(
          var_name + "_imag", &mfem_variable.getComplexGridFunction()->imag());
    }
  }
  else // must be real, but may have been set up indirectly from a MOOSE variable
  {
    Variable & mfem_variable = getMFEMObject<Variable>("MooseVariableBase", var_name);
    getProblemData().gridfunctions.Register(var_name, mfem_variable.getGridFunction());
    if (mfem_variable.isScalar())
      getCoefficients().declareScalar<mfem::GridFunctionCoefficient>(
          var_name, mfem_variable.getGridFunction().get());
    else
      getCoefficients().declareVector<mfem::VectorGridFunctionCoefficient>(
          var_name, mfem_variable.getGridFunction().get());
  }
}

void
Problem::addAuxVariable(const std::string & var_type,
                        const std::string & var_name,
                        InputParameters & parameters)
{
  // We handle MFEM AuxVariables just like MFEM Variables, except
  // we do not add additional GridFunctions for time derivatives.
  addGridFunction(var_type, var_name, parameters);
}

void
Problem::addAuxKernel(const std::string & kernel_name,
                      const std::string & name,
                      InputParameters & parameters)
{
  addObject<ExecutedObject>(kernel_name, name, parameters);
}

void
Problem::addKernel(const std::string & kernel_name,
                   const std::string & name,
                   InputParameters & parameters)
{
  auto kernel = addObject<Kernel>(kernel_name, name, parameters).front();
  const auto & kernel_object = *kernel;

  if (dynamic_cast<const ComplexKernel *>(&kernel_object))
  {
    auto complex_kernel = std::dynamic_pointer_cast<ComplexKernel>(kernel);
    auto eqsys = std::dynamic_pointer_cast<ComplexEquationSystem>(getProblemData().eqn_system);
    if (eqsys)
      eqsys->AddComplexKernel(std::move(complex_kernel));
    else
      mooseError("Cannot add complex kernel with name '" + name +
                 "' because there is no corresponding equation system.");
  }
  else
  {
    auto eqsys = std::dynamic_pointer_cast<EquationSystem>(getProblemData().eqn_system);
    if (eqsys)
      eqsys->AddKernel(std::move(kernel));
    else
      mooseError("Cannot add kernel with name '" + name +
                 "' because there is no corresponding equation system.");
  }
}

void
Problem::addRealComponentToKernel(const std::string & kernel_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  auto parent_ptr = std::dynamic_pointer_cast<ComplexKernel>(
      getMFEMObject<ComplexKernel>("Kernel", name).getSharedPtr());
  parameters.set<VariableName>("variable") = parent_ptr->getParam<VariableName>("variable");
  auto kernel_ptr = addObject<Kernel>(kernel_name, name + "_real", parameters).front();
  parent_ptr->setRealKernel(kernel_ptr);
}

void
Problem::addImagComponentToKernel(const std::string & kernel_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  auto parent_ptr = std::dynamic_pointer_cast<ComplexKernel>(
      getMFEMObject<ComplexKernel>("Kernel", name).getSharedPtr());
  parameters.set<VariableName>("variable") = parent_ptr->getParam<VariableName>("variable");
  auto kernel_ptr = addObject<Kernel>(kernel_name, name + "_imag", parameters).front();
  parent_ptr->setImagKernel(kernel_ptr);
}

void
Problem::addRealComponentToBC(const std::string & kernel_name,
                              const std::string & name,
                              InputParameters & parameters)
{
  auto parent_ptr = std::dynamic_pointer_cast<ComplexIntegratedBC>(
      getMFEMObject<ComplexIntegratedBC>("BoundaryCondition", name).getSharedPtr());
  parameters.set<VariableName>("variable") = parent_ptr->getParam<VariableName>("variable");
  parameters.set<std::vector<BoundaryName>>("boundary") =
      parent_ptr->getParam<std::vector<BoundaryName>>("boundary");
  auto bc_ptr = std::dynamic_pointer_cast<IntegratedBC>(
      addObject<BoundaryCondition>(kernel_name, name + "_real", parameters).front());
  parent_ptr->setRealBC(bc_ptr);
}

void
Problem::addImagComponentToBC(const std::string & kernel_name,
                              const std::string & name,
                              InputParameters & parameters)
{
  auto parent_ptr = std::dynamic_pointer_cast<ComplexIntegratedBC>(
      getMFEMObject<ComplexIntegratedBC>("BoundaryCondition", name).getSharedPtr());
  parameters.set<VariableName>("variable") = parent_ptr->getParam<VariableName>("variable");
  parameters.set<std::vector<BoundaryName>>("boundary") =
      parent_ptr->getParam<std::vector<BoundaryName>>("boundary");
  auto bc_ptr = std::dynamic_pointer_cast<IntegratedBC>(
      addObject<BoundaryCondition>(kernel_name, name + "_imag", parameters).front());
  parent_ptr->setImagBC(bc_ptr);
}

libMesh::Point
pointFromMFEMVector(const mfem::Vector & vec)
{
  return libMesh::Point(
      vec.Elem(0), vec.Size() > 1 ? vec.Elem(1) : 0., vec.Size() > 2 ? vec.Elem(2) : 0.);
}

int
vectorFunctionDim(const std::string & type, const InputParameters & parameters)
{
  if (type == "LevelSetOlssonVortex")
  {
    return 2;
  }
  else if (type == "ParsedVectorFunction")
  {
    if (parameters.isParamSetByUser("expression_z") || parameters.isParamSetByUser("value_z"))
    {
      return 3;
    }
    else if (parameters.isParamSetByUser("expression_y") || parameters.isParamSetByUser("value_y"))
    {
      return 2;
    }
    else
    {
      return 1;
    }
  }
  else
  {
    return 3;
  }
}

const std::vector<std::string> SCALAR_FUNCS = {"Axisymmetric2D3DSolutionFunction",
                                               "BicubicSplineFunction",
                                               "CoarsenedPiecewiseLinear",
                                               "CompositeFunction",
                                               "ConstantFunction",
                                               "ImageFunction",
                                               "ParsedFunction",
                                               "ParsedGradFunction",
                                               "PeriodicFunction",
                                               "PiecewiseBilinear",
                                               "PiecewiseConstant",
                                               "PiecewiseConstantFromCSV",
                                               "PiecewiseLinear",
                                               "PiecewiseLinearFromVectorPostprocessor",
                                               "PiecewiseMultiInterpolation",
                                               "PiecewiseMulticonstant",
                                               "SolutionFunction",
                                               "SplineFunction",
                                               "FunctionSeries",
                                               "LevelSetOlssonBubble",
                                               "LevelSetOlssonPlane",
                                               "NearestReporterCoordinatesFunction",
                                               "ParameterMeshFunction",
                                               "ParsedOptimizationFunction",
                                               "FourierNoise",
                                               "MovingPlanarFront",
                                               "MultiControlDrumFunction",
                                               "Grad2ParsedFunction",
                                               "GradParsedFunction",
                                               "ScaledAbsDifferenceDRLRewardFunction",
                                               "CircularAreaHydraulicDiameterFunction",
                                               "CosineHumpFunction",
                                               "CosineTransitionFunction",
                                               "CubicTransitionFunction",
                                               "GeneralizedCircumference",
                                               "PiecewiseFunction",
                                               "TimeRampFunction"},
                               VECTOR_FUNCS = {"ParsedVectorFunction", "LevelSetOlssonVortex"};

void
Problem::addFunction(const std::string & type,
                     const std::string & name,
                     InputParameters & parameters)
{
  ExternalProblem::addFunction(type, name, parameters);
  auto & func = getFunction(name);
  // FIXME: Do we want to have optimised versions for when functions
  // are only of space or only of time.
  if (std::find(SCALAR_FUNCS.begin(), SCALAR_FUNCS.end(), type) != SCALAR_FUNCS.end())
  {
    getCoefficients().declareScalar<mfem::FunctionCoefficient>(
        name,
        [&func](const mfem::Vector & p, mfem::real_t t) -> mfem::real_t
        { return func.value(t, pointFromMFEMVector(p)); });
  }
  else if (std::find(VECTOR_FUNCS.begin(), VECTOR_FUNCS.end(), type) != VECTOR_FUNCS.end())
  {
    int dim = vectorFunctionDim(type, parameters);
    getCoefficients().declareVector<mfem::VectorFunctionCoefficient>(
        name,
        dim,
        [&func, dim](const mfem::Vector & p, mfem::real_t t, mfem::Vector & u)
        {
          libMesh::RealVectorValue vector_value = func.vectorValue(t, pointFromMFEMVector(p));
          for (int i = 0; i < dim; i++)
          {
            u[i] = vector_value(i);
          }
        });
  }
  else if ("MFEMParsedFunction" != type)
  {
    mooseWarning("Could not identify whether function ",
                 type,
                 " is scalar or vector; no MFEM coefficient object created.");
  }
}

void
Problem::addPostprocessor(const std::string & type,
                          const std::string & name,
                          InputParameters & parameters)
{
  if (parameters.getSystemAttributeName() == "Moose::MFEM::ExecutedObject")
  {
    checkUserObjectNameCollision(name, "Postprocessor");
    addObject<ExecutedObject>(type, name, parameters);
    const PostprocessorValue & val = getPostprocessorValueByName(name);
    getCoefficients().declareScalar<mfem::FunctionCoefficient>(
        name, [&val](const mfem::Vector &) -> mfem::real_t { return val; });
  }
  else
    ExternalProblem::addPostprocessor(type, name, parameters);
}

void
Problem::addVectorPostprocessor(const std::string & type,
                                const std::string & name,
                                InputParameters & parameters)
{
  if (parameters.getSystemAttributeName() == "Moose::MFEM::ExecutedObject")
  {
    checkUserObjectNameCollision(name, "VectorPostprocessor");
    addObject<ExecutedObject>(type, name, parameters);
  }
  else
    FEProblemBase::addVectorPostprocessor(type, name, parameters);
}

InputParameters
Problem::addMFEMFESpaceFromMOOSEVariable(InputParameters & parameters)
{

  InputParameters fespace_params = _factory.getValidParams("MFEMGenericFESpace");
  InputParameters mfem_variable_params = _factory.getValidParams("MFEMVariable");

  auto moose_fe_type =
      FEType(Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")),
             Utility::string_to_enum<FEFamily>(parameters.get<MooseEnum>("family")));

  std::string mfem_family;
  int mfem_vdim = 1;

  switch (moose_fe_type.family)
  {
    case FEFamily::LAGRANGE:
      mfem_family = "H1";
      mfem_vdim = 1;
      break;
    case FEFamily::LAGRANGE_VEC:
      mfem_family = "H1";
      mfem_vdim = 3;
      break;
    case FEFamily::MONOMIAL:
      mfem_family = "L2";
      mfem_vdim = 1;
      break;
    case FEFamily::MONOMIAL_VEC:
      mfem_family = "L2";
      mfem_vdim = 3;
      break;
    default:
      mooseError("Unable to set MFEM FESpace for MOOSE variable");
      break;
  }

  // Create fespace name. If this already exists, we will reuse this for
  // the mfem variable ("gridfunction").
  const std::string fespace_name = mfem_family + "_" +
                                   std::to_string(mesh().getMFEMParMesh().Dimension()) + "D_P" +
                                   std::to_string(moose_fe_type.order.get_order());

  // Set all fespace parameters.
  fespace_params.set<std::string>("fec_name") = fespace_name;
  fespace_params.set<int>("vdim") = mfem_vdim;

  if (!hasMFEMObject("Moose::MFEM::FESpace", fespace_name)) // Create the fespace (implicit).
  {
    addFESpace("MFEMGenericFESpace", fespace_name, fespace_params);
  }

  mfem_variable_params.set<Moose::MFEM::FESpaceName>("fespace") = fespace_name;

  return mfem_variable_params;
}

void
Problem::displaceMesh()
{
  // Displace mesh
  if (mesh().shouldDisplace())
  {
    mesh().displace(static_cast<mfem::GridFunction const &>(*getMeshDisplacementGridFunction()));
    // TODO: update FESpaces GridFunctions etc for transient solves
  }
}

std::optional<std::reference_wrapper<mfem::ParGridFunction const>>
Problem::getMeshDisplacementGridFunction()
{
  // If C++23 transform were available this would be easier
  auto const displacement_variable = mesh().getMeshDisplacementVariable();
  if (displacement_variable)
  {
    return *_problem_data.gridfunctions.Get(displacement_variable.value());
  }
  else
  {
    return std::nullopt;
  }
}

void
Problem::rebalanceMesh(mfem::ParMesh & pmesh)
{
  if (pmesh.Nonconforming())
  {
    pmesh.Rebalance();
    updateFESpaces();
    updateGridFunctions();
  }
}

void
Problem::updateFESpaces()
{
  for (const auto & fe_space_pair : _problem_data.fespaces)
    fe_space_pair.second->Update();
}

void
Problem::updateGridFunctions()
{
  for (const auto & gridfunction_pair : _problem_data.gridfunctions)
    gridfunction_pair.second->Update();
}

std::vector<VariableName>
Problem::getAuxVariableNames()
{
  return systemBaseAuxiliary().getVariableNames();
}

Mesh &
Problem::mesh()
{
  mooseAssert(ExternalProblem::mesh().type() == "MFEMMesh",
              "Please choose the MFEMMesh mesh type for an Moose::MFEM::Problem\n");
  return static_cast<Mesh &>(_mesh);
}

const Mesh &
Problem::mesh() const
{
  return const_cast<Problem *>(this)->mesh();
}

void
Problem::addSubMesh(const std::string & var_type,
                    const std::string & var_name,
                    InputParameters & parameters)
{
  auto & mfem_submesh = *addObject<SubMesh>(var_type, var_name, parameters).front();
  // Register submesh.
  getProblemData().submeshes.Register(var_name, mfem_submesh.getSubMesh());
}

void
Problem::addTransfer(const std::string & transfer_name,
                     const std::string & name,
                     InputParameters & parameters)
{
  if (parameters.getBase() == "Moose::MFEM::SubMeshTransfer")
    addObject<ExecutedObject>(transfer_name, name, parameters);
  else
    FEProblemBase::addTransfer(transfer_name, name, parameters);
}

void
Problem::addInitialCondition(const std::string & ic_name,
                             const std::string & name,
                             InputParameters & parameters)
{
  addObject<ExecutedObject>(ic_name, name, parameters);
}

void
Problem::executeMFEMObjects(const ExecFlagType & exec_type)
{
  std::vector<ExecutedObject *> objects;
  theWarehouse()
      .query()
      .condition<AttribSystem>("Moose::MFEM::ExecutedObject")
      .condition<AttribExecOns>(exec_type)
      .condition<AttribThread>(0)
      .queryInto(objects);

  std::map<std::string, const ExecutedObject *> suppliers;
  for (auto * const object : objects)
    for (const auto & item : object->getSuppliedItems())
    {
      const auto [it, inserted] = suppliers.emplace(item, object);
      if (!inserted && it->second != object)
        mooseError("MFEM executed-object dependency ambiguity on ",
                   exec_type,
                   ": both '",
                   it->second->name(),
                   "' and '",
                   object->name(),
                   "' supply '",
                   item,
                   "'.");
    }

  for (auto * const object : objects)
  {
    object->initialize();
    object->execute();
    object->finalize();

    if (auto * const pp = dynamic_cast<const Postprocessor *>(object))
    {
      _reporter_data.finalize(pp->PPName());
      setPostprocessorValueByName(pp->PPName(), pp->getValue());
    }

    if (auto * const vpp = dynamic_cast<VectorPostprocessor *>(object))
      _reporter_data.finalize(vpp->PPName());
  }
}

std::string
Problem::solverTypeString(const unsigned int libmesh_dbg_var(solver_sys_num))
{
  mooseAssert(solver_sys_num == 0, "No support for multi-system with MFEM right now");
  const auto & solver = getProblemData().jacobian_solver;
  if (!solver)
  {
    mooseAssert(!shouldSolve(), "jacobian_solver is null but Problem/solve is not false");
    return "none";
  }
  return solver->type();
}

bool
Problem::hasMFEMObject(const std::string & system, const std::string & name) const
{
  std::vector<MooseObject *> objs;
  theWarehouse()
      .query()
      .condition<AttribSystem>(system)
      .condition<AttribThread>(0)
      .condition<AttribName>(name)
      .queryInto(objs);
  return !objs.empty();
}

} // namespace Moose::MFEM
#endif
