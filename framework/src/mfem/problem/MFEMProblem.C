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
#include "MFEMVariable.h"
#include "MFEMIndicator.h"
#include "MFEMSubMesh.h"
#include "MFEMFunctorMaterial.h"
#include "MFEMExecutedObject.h"
#include "MFEMVectorUtils.h"
#include "libmesh/string_to_enum.h"

#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <deque>
#include <sstream>

registerMooseObject("MooseApp", MFEMProblem);

InputParameters
MFEMProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addClassDescription("Problem type for building and solving the finite element problem "
                             "using the MFEM finite element library.");
  MooseEnum numeric_types("real complex", "real");
  params.addParam<MooseEnum>("numeric_type", numeric_types, "Number type used for the problem");

  return params;
}

MFEMProblem::MFEMProblem(const InputParameters & params)
  : ExternalProblem(params), _num_type{static_cast<int>(getParam<MooseEnum>("numeric_type"))}
{
  // Initialise Hypre for all MFEM problems.
  mfem::Hypre::Init();
  // Disable multithreading for all MFEM problems (including any libMesh or MFEM subapps).
  libMesh::libMeshPrivateData::_n_threads = 1;
#ifdef LIBMESH_HAVE_OPENMP
  omp_set_num_threads(1);
#endif
  setMesh();
}

void
MFEMProblem::initialSetup()
{
  ExternalProblem::initialSetup();

  // MFEM indicators create their estimators during addIndicator(); markers still need an explicit
  // setup pass because they are no longer initialized through the libMesh/MOOSE user-object path.
  std::vector<MFEMRefinementMarker *> markers;
  theWarehouse().query().condition<AttribSystem>("Marker").queryInto(markers);
  for (auto marker : markers)
    marker->initialSetup();
}

void
MFEMProblem::execute(const ExecFlagType & exec_type)
{
  setCurrentExecuteOnFlag(exec_type);
  executeMFEMObjects(exec_type);

  ExternalProblem::execute(exec_type);
}

void
MFEMProblem::setMesh()
{
  auto pmesh = mesh().getMFEMParMeshPtr();
  getProblemData().pmesh = pmesh;
  getProblemData().comm = pmesh->GetComm();
  getProblemData().num_procs = pmesh->GetNRanks();
  getProblemData().myid = pmesh->GetMyRank();
}

void
MFEMProblem::addMFEMPreconditioner(const std::string & user_object_name,
                                   const std::string & name,
                                   InputParameters & parameters)
{
  addObject<MFEMSolverBase>(user_object_name, name, parameters);
}

void
MFEMProblem::addIndicator(const std::string & user_object_name,
                          const std::string & name,
                          InputParameters & parameters)
{
  auto estimator = addObject<MFEMIndicator>(user_object_name, name, parameters).front();

  // construct the estimator itself
  estimator->createEstimator();
}

void
MFEMProblem::addMarker(const std::string & user_object_name,
                       const std::string & name,
                       InputParameters & parameters)
{
  getProblemData().refiner =
      addObject<MFEMRefinementMarker>(user_object_name, name, parameters).front();
}

void
MFEMProblem::addMFEMSolver(const std::string & user_object_name,
                           const std::string & name,
                           InputParameters & parameters)
{
  getProblemData().jacobian_solver =
      addObject<MFEMSolverBase>(user_object_name, name, parameters).front();
}

void
MFEMProblem::addMFEMNonlinearSolver(unsigned int nl_max_its,
                                    mfem::real_t nl_abs_tol,
                                    mfem::real_t nl_rel_tol,
                                    unsigned int print_level)
{
  // TODO: allow users to specify other mfem::IterativeSolvers
  auto nl_solver = std::make_shared<mfem::NewtonSolver>(getComm());

  // Defaults to one iteration, without further nonlinear iterations
  nl_solver->SetRelTol(nl_rel_tol);
  nl_solver->SetAbsTol(nl_abs_tol);
  nl_solver->SetMaxIter(nl_max_its);
  nl_solver->SetPrintLevel(print_level);
  getProblemData().nonlinear_solver = nl_solver;
}

void
MFEMProblem::addBoundaryCondition(const std::string & bc_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  auto bc = addObject<MFEMBoundaryCondition>(bc_name, name, parameters).front();
  const auto & mfem_bc = *bc;

  if (dynamic_cast<const MFEMIntegratedBC *>(&mfem_bc))
  {
    auto integrated_bc = std::dynamic_pointer_cast<MFEMIntegratedBC>(bc);
    auto eqsys =
        std::dynamic_pointer_cast<Moose::MFEM::EquationSystem>(getProblemData().eqn_system);
    if (eqsys)
      eqsys->AddIntegratedBC(std::move(integrated_bc));
    else
      mooseError("Cannot add integrated BC with name '" + name +
                 "' because there is no corresponding equation system.");
  }
  else if (dynamic_cast<const MFEMComplexIntegratedBC *>(&mfem_bc))
  {
    auto integrated_bc = std::dynamic_pointer_cast<MFEMComplexIntegratedBC>(bc);
    auto eqsys =
        std::dynamic_pointer_cast<Moose::MFEM::ComplexEquationSystem>(getProblemData().eqn_system);
    if (eqsys)
      eqsys->AddComplexIntegratedBC(std::move(integrated_bc));
    else
      mooseError("Cannot add complex integrated BC with name '" + name +
                 "' because there is no corresponding equation system.");
  }
  else if (dynamic_cast<const MFEMComplexEssentialBC *>(&mfem_bc))
  {
    auto essential_bc = std::dynamic_pointer_cast<MFEMComplexEssentialBC>(bc);
    auto eqsys =
        std::dynamic_pointer_cast<Moose::MFEM::ComplexEquationSystem>(getProblemData().eqn_system);
    if (eqsys)
      eqsys->AddComplexEssentialBCs(std::move(essential_bc));
    else
      mooseError("Cannot add boundary condition with name '" + name +
                 "' because there is no corresponding equation system.");
  }
  else if (dynamic_cast<const MFEMEssentialBC *>(&mfem_bc))
  {
    auto essential_bc = std::dynamic_pointer_cast<MFEMEssentialBC>(bc);
    auto eqsys =
        std::dynamic_pointer_cast<Moose::MFEM::EquationSystem>(getProblemData().eqn_system);
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
MFEMProblem::addMaterial(const std::string &, const std::string &, InputParameters &)
{
  mooseError(
      "MFEM materials must be added through the 'FunctorMaterials' block and not 'Materials'");
}

void
MFEMProblem::addFunctorMaterial(const std::string & material_name,
                                const std::string & name,
                                InputParameters & parameters)
{
  addObject<MFEMFunctorMaterial>(material_name, name, parameters);
}

void
MFEMProblem::addFESpace(const std::string & type,
                        const std::string & name,
                        InputParameters & parameters)
{
  auto & mfem_fespace = *addObject<MFEMFESpace>(type, name, parameters).front();

  // Register fespace and associated fe collection.
  getProblemData().fecs.Register(name, mfem_fespace.getFEC());
  getProblemData().fespaces.Register(name, mfem_fespace.getFESpace());
}

void
MFEMProblem::addVariable(const std::string & var_type,
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
        getMFEMObject<MFEMVariable>("MooseVariableBase", var_name).getTimeDerivativeName();
    getProblemData().time_derivative_map.addTimeDerivativeAssociation(var_name,
                                                                      time_derivative_var_name);
    addGridFunction(var_type, time_derivative_var_name, parameters);
  }
}

void
MFEMProblem::addGridFunction(const std::string & var_type,
                             const std::string & var_name,
                             InputParameters & parameters)
{

  if (var_type == "MFEMVariable" || var_type == "MFEMComplexVariable")
  {
    // Add MFEM variable directly.
    if (var_type == "MFEMComplexVariable")
      addObject<MFEMComplexVariable>(var_type, var_name, parameters);
    else
      addObject<MFEMVariable>(var_type, var_name, parameters);
  }
  else
  {
    // Add MOOSE variable.
    ExternalProblem::addVariable(var_type, var_name, parameters);

    // Add MFEM variable indirectly ("gridfunction").
    InputParameters mfem_variable_params = addMFEMFESpaceFromMOOSEVariable(parameters);
    addObject<MFEMVariable>("MFEMVariable", var_name, mfem_variable_params);
  }

  // Register gridfunction.
  if (var_type == "MFEMComplexVariable")
  {
    MFEMComplexVariable & mfem_variable =
        getMFEMObject<MFEMComplexVariable>("MooseVariableBase", var_name);
    getProblemData().cmplx_gridfunctions.Register(var_name, mfem_variable.getComplexGridFunction());
    mfem_variable.declareCoefficients();
  }
  else // must be real, but may have been set up indirectly from a MOOSE variable
  {
    MFEMVariable & mfem_variable = getMFEMObject<MFEMVariable>("MooseVariableBase", var_name);
    getProblemData().gridfunctions.Register(var_name, mfem_variable.getGridFunction());
    mfem_variable.declareCoefficients();
  }
}

void
MFEMProblem::addAuxVariable(const std::string & var_type,
                            const std::string & var_name,
                            InputParameters & parameters)
{
  // We handle MFEM AuxVariables just like MFEM Variables, except
  // we do not add additional GridFunctions for time derivatives.
  addGridFunction(var_type, var_name, parameters);
}

void
MFEMProblem::addAuxKernel(const std::string & kernel_name,
                          const std::string & name,
                          InputParameters & parameters)
{
  addObject<MFEMExecutedObject>(kernel_name, name, parameters);
}

void
MFEMProblem::addKernel(const std::string & kernel_name,
                       const std::string & name,
                       InputParameters & parameters)
{
  auto kernel = addObject<MFEMKernel>(kernel_name, name, parameters).front();
  const auto & kernel_object = *kernel;

  if (dynamic_cast<const MFEMComplexKernel *>(&kernel_object))
  {
    auto complex_kernel = std::dynamic_pointer_cast<MFEMComplexKernel>(kernel);
    auto eqsys =
        std::dynamic_pointer_cast<Moose::MFEM::ComplexEquationSystem>(getProblemData().eqn_system);
    if (eqsys)
      eqsys->AddComplexKernel(std::move(complex_kernel));
    else
      mooseError("Cannot add complex kernel with name '" + name +
                 "' because there is no corresponding equation system.");
  }
  else
  {
    auto eqsys =
        std::dynamic_pointer_cast<Moose::MFEM::EquationSystem>(getProblemData().eqn_system);
    if (eqsys)
      eqsys->AddKernel(std::move(kernel));
    else
      mooseError("Cannot add kernel with name '" + name +
                 "' because there is no corresponding equation system.");
  }
}

void
MFEMProblem::addRealComponentToKernel(const std::string & kernel_name,
                                      const std::string & name,
                                      InputParameters & parameters)
{
  auto parent_ptr = std::dynamic_pointer_cast<MFEMComplexKernel>(
      getMFEMObject<MFEMComplexKernel>("Kernel", name).getSharedPtr());
  parameters.set<VariableName>("variable") = parent_ptr->getParam<VariableName>("variable");
  auto kernel_ptr = addObject<MFEMKernel>(kernel_name, name + "_real", parameters).front();
  parent_ptr->setRealKernel(kernel_ptr);
}

void
MFEMProblem::addImagComponentToKernel(const std::string & kernel_name,
                                      const std::string & name,
                                      InputParameters & parameters)
{
  auto parent_ptr = std::dynamic_pointer_cast<MFEMComplexKernel>(
      getMFEMObject<MFEMComplexKernel>("Kernel", name).getSharedPtr());
  parameters.set<VariableName>("variable") = parent_ptr->getParam<VariableName>("variable");
  auto kernel_ptr = addObject<MFEMKernel>(kernel_name, name + "_imag", parameters).front();
  parent_ptr->setImagKernel(kernel_ptr);
}

void
MFEMProblem::addRealComponentToBC(const std::string & kernel_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  auto parent_ptr = std::dynamic_pointer_cast<MFEMComplexIntegratedBC>(
      getMFEMObject<MFEMComplexIntegratedBC>("BoundaryCondition", name).getSharedPtr());
  parameters.set<VariableName>("variable") = parent_ptr->getParam<VariableName>("variable");
  parameters.set<std::vector<BoundaryName>>("boundary") =
      parent_ptr->getParam<std::vector<BoundaryName>>("boundary");
  auto bc_ptr = std::dynamic_pointer_cast<MFEMIntegratedBC>(
      addObject<MFEMBoundaryCondition>(kernel_name, name + "_real", parameters).front());
  parent_ptr->setRealBC(bc_ptr);
}

void
MFEMProblem::addImagComponentToBC(const std::string & kernel_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  auto parent_ptr = std::dynamic_pointer_cast<MFEMComplexIntegratedBC>(
      getMFEMObject<MFEMComplexIntegratedBC>("BoundaryCondition", name).getSharedPtr());
  parameters.set<VariableName>("variable") = parent_ptr->getParam<VariableName>("variable");
  parameters.set<std::vector<BoundaryName>>("boundary") =
      parent_ptr->getParam<std::vector<BoundaryName>>("boundary");
  auto bc_ptr = std::dynamic_pointer_cast<MFEMIntegratedBC>(
      addObject<MFEMBoundaryCondition>(kernel_name, name + "_imag", parameters).front());
  parent_ptr->setImagBC(bc_ptr);
}

int
vectorFunctionDim(const std::string & type, const InputParameters & parameters)
{
  if (parameters.isParamSetByUser("expression_z"))
    return 3;
  if (parameters.isParamSetByUser("expression_y") || type == "LevelSetOlssonVortex")
    return 2;
  if (parameters.isParamSetByUser("expression_x"))
    return 1;

  return 3;
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
MFEMProblem::addFunction(const std::string & type,
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
        { return func.value(t, Moose::MFEM::libMeshPointFromMFEMVector(p)); });
  }
  else if (std::find(VECTOR_FUNCS.begin(), VECTOR_FUNCS.end(), type) != VECTOR_FUNCS.end())
  {
    int dim = vectorFunctionDim(type, parameters);
    getCoefficients().declareVector<mfem::VectorFunctionCoefficient>(
        name,
        dim,
        [&func, dim](const mfem::Vector & p, mfem::real_t t, mfem::Vector & u)
        {
          libMesh::RealVectorValue vector_value =
              func.vectorValue(t, Moose::MFEM::libMeshPointFromMFEMVector(p));
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
MFEMProblem::addPostprocessor(const std::string & type,
                              const std::string & name,
                              InputParameters & parameters)
{
  if (parameters.getSystemAttributeName() == "MFEMExecutedObject")
  {
    checkUserObjectNameCollision(name, "Postprocessor");
    addObject<MFEMExecutedObject>(type, name, parameters);
    const PostprocessorValue & val = getPostprocessorValueByName(name);
    getCoefficients().declareScalar<mfem::FunctionCoefficient>(
        name, [&val](const mfem::Vector &) -> mfem::real_t { return val; });
  }
  else
    ExternalProblem::addPostprocessor(type, name, parameters);
}

void
MFEMProblem::addVectorPostprocessor(const std::string & type,
                                    const std::string & name,
                                    InputParameters & parameters)
{
  if (parameters.getSystemAttributeName() == "MFEMExecutedObject")
  {
    checkUserObjectNameCollision(name, "VectorPostprocessor");
    addObject<MFEMExecutedObject>(type, name, parameters);
  }
  else
    ExternalProblem::addVectorPostprocessor(type, name, parameters);
}

InputParameters
MFEMProblem::addMFEMFESpaceFromMOOSEVariable(InputParameters & parameters)
{

  InputParameters fespace_params = _factory.getValidParams("MFEMGenericFESpace");
  InputParameters variable_params = _factory.getValidParams("MFEMVariable");

  const auto family = Utility::string_to_enum<FEFamily>(parameters.get<MooseEnum>("family"));
  auto order = static_cast<int>(parameters.get<MooseEnum>("order"));
  const auto dim = mesh().dimension();

  std::string space;
  int vdim = 1;

  switch (family)
  {
    case FEFamily::LAGRANGE:
      space = "H1";
      break;
    case FEFamily::NEDELEC_ONE:
      space = "ND";
      break;
    case FEFamily::RAVIART_THOMAS:
      space = "RT";
      --order;
      break;
    case FEFamily::MONOMIAL:
    case FEFamily::L2_LAGRANGE:
      space = "L2";
      break;
    case FEFamily::LAGRANGE_VEC:
      space = "H1";
      vdim = dim;
      break;
    case FEFamily::MONOMIAL_VEC:
    case FEFamily::L2_LAGRANGE_VEC:
      space = "L2";
      vdim = dim;
      break;
    default:
      mooseError("Unable to set MFEM FESpace for MOOSE variable");
      break;
  }

  // Create fespace name. If this already exists, we will reuse this for
  // the mfem variable ("gridfunction"). If using AMR, this implies all
  // variables sharing the fespace are affected.
  const auto fec_name = space + "_" + std::to_string(dim) + "D_P" + std::to_string(order);
  const auto fes_name = fec_name + "_X" + std::to_string(vdim);

  // Set all fespace parameters.
  fespace_params.set<std::string>("fec_name") = fec_name;
  fespace_params.set<int>("vdim") = vdim;

  if (!hasMFEMObject("MFEMFESpace", fes_name))
    addFESpace("MFEMGenericFESpace", fes_name, fespace_params);

  variable_params.set<MFEMFESpaceName>("fespace") = fes_name;

  return variable_params;
}

void
MFEMProblem::displaceMesh()
{
  // Displace mesh
  if (mesh().shouldDisplace())
  {
    mesh().displace(static_cast<mfem::GridFunction const &>(*getMeshDisplacementGridFunction()));
    // TODO: update FESpaces GridFunctions etc for transient solves
  }
}

std::optional<std::reference_wrapper<mfem::ParGridFunction const>>
MFEMProblem::getMeshDisplacementGridFunction()
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
MFEMProblem::rebalanceMesh(mfem::ParMesh & pmesh)
{
  if (pmesh.Nonconforming())
  {
    pmesh.Rebalance();
    updateFESpaces();
    updateGridFunctions();
  }
}

void
MFEMProblem::updateFESpaces()
{
  for (const auto & fe_space_pair : _problem_data.fespaces)
    fe_space_pair.second->Update();
}

void
MFEMProblem::updateGridFunctions()
{
  for (const auto & gridfunction_pair : _problem_data.gridfunctions)
    gridfunction_pair.second->Update();
}

std::vector<VariableName>
MFEMProblem::getAuxVariableNames()
{
  return systemBaseAuxiliary().getVariableNames();
}

MFEMMesh &
MFEMProblem::mesh()
{
  mooseAssert(ExternalProblem::mesh().type() == "MFEMMesh",
              "Please choose the MFEMMesh mesh type for an MFEMProblem\n");
  return static_cast<MFEMMesh &>(_mesh);
}

const MFEMMesh &
MFEMProblem::mesh() const
{
  return const_cast<MFEMProblem *>(this)->mesh();
}

void
MFEMProblem::addSubMesh(const std::string & var_type,
                        const std::string & var_name,
                        InputParameters & parameters)
{
  auto & mfem_submesh = *addObject<MFEMSubMesh>(var_type, var_name, parameters).front();
  // Register submesh.
  getProblemData().submeshes.Register(var_name, mfem_submesh.getSubMesh());
}

void
MFEMProblem::addTransfer(const std::string & transfer_name,
                         const std::string & name,
                         InputParameters & parameters)
{
  if (parameters.getBase() == "MFEMSubMeshTransfer")
    addObject<MFEMExecutedObject>(transfer_name, name, parameters);
  else
    ExternalProblem::addTransfer(transfer_name, name, parameters);
}

void
MFEMProblem::addInitialCondition(const std::string & ic_name,
                                 const std::string & name,
                                 InputParameters & parameters)
{
  addObject<MFEMExecutedObject>(ic_name, name, parameters);
}

void
MFEMProblem::executeMFEMObjects(const ExecFlagType & exec_type)
{
  std::vector<MFEMExecutedObject *> objects;
  theWarehouse()
      .query()
      .condition<AttribSystem>("MFEMExecutedObject")
      .condition<AttribExecOns>(exec_type)
      .condition<AttribThread>(0)
      .queryInto(objects);

  std::map<std::string, const MFEMExecutedObject *> suppliers;
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
MFEMProblem::solverTypeString(const unsigned int libmesh_dbg_var(solver_sys_num))
{
  mooseAssert(solver_sys_num == 0, "No support for multi-system with MFEM right now");

  std::vector<std::string> solvers;

  if (getProblemData().nonlinear_solver)
    solvers.push_back(MooseUtils::prettyCppType(getProblemData().nonlinear_solver.get()));

  if (getProblemData().jacobian_solver)
  {
    solvers.push_back(MooseUtils::prettyCppType(getProblemData().jacobian_solver.get()));
    if (const auto * prec = getProblemData().jacobian_solver->getPreconditioner())
      solvers.push_back(MooseUtils::prettyCppType(prec));
  }

  return solvers.empty() ? "None" : MooseUtils::stringJoin(solvers);
}

bool
MFEMProblem::hasMFEMObject(const std::string & system, const std::string & name) const
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

#endif
