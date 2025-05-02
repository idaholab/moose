#ifdef MFEM_ENABLED

#include "MFEMProblem.h"

#include <vector>
#include <algorithm>

registerMooseObject("MooseApp", MFEMProblem);

InputParameters
MFEMProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addClassDescription("Problem type for building and solving finite element problem using"
                             " the MFEM finite element library.");
  return params;
}

MFEMProblem::MFEMProblem(const InputParameters & params) : ExternalProblem(params) {}

void
MFEMProblem::initialSetup()
{
  FEProblemBase::initialSetup();
  addMFEMNonlinearSolver();
}

void
MFEMProblem::setMesh()
{
  auto pmesh = mesh().getMFEMParMeshPtr();
  getProblemData().pmesh = pmesh;
  getProblemData().comm = pmesh->GetComm();
  MPI_Comm_size(pmesh->GetComm(), &(getProblemData().num_procs));
  MPI_Comm_rank(pmesh->GetComm(), &(getProblemData().myid));
}

void
MFEMProblem::initProblemOperator()
{
  setMesh();
  auto mfem_exec_ptr = dynamic_cast<MFEMExecutioner *>(_app.getExecutioner());
  if (mfem_exec_ptr != nullptr)
  {
    mfem_exec_ptr->constructProblemOperator();
  }
  else
  {
    mooseError("Executioner used that is not currently supported by MFEMProblem");
  }
}

void
MFEMProblem::addMFEMPreconditioner(const std::string & user_object_name,
                                   const std::string & name,
                                   InputParameters & parameters)
{
  FEProblemBase::addUserObject(user_object_name, name, parameters);
  auto object_ptr = getUserObject<MFEMSolverBase>(name).getSharedPtr();

  getProblemData().mfem_preconditioner = std::dynamic_pointer_cast<MFEMSolverBase>(object_ptr);
}

void
MFEMProblem::addMFEMSolver(const std::string & user_object_name,
                           const std::string & name,
                           InputParameters & parameters)
{
  FEProblemBase::addUserObject(user_object_name, name, parameters);
  auto object_ptr = getUserObject<MFEMSolverBase>(name).getSharedPtr();

  getProblemData().mfem_solver = std::dynamic_pointer_cast<MFEMSolverBase>(object_ptr);
}

void
MFEMProblem::addMFEMNonlinearSolver()
{
  auto nl_solver = std::make_shared<mfem::NewtonSolver>(getProblemData().comm);

  // Defaults to one iteration, without further nonlinear iterations
  nl_solver->SetRelTol(0.0);
  nl_solver->SetAbsTol(0.0);
  nl_solver->SetMaxIter(1);

  getProblemData().nonlinear_solver = nl_solver;
}

void
MFEMProblem::addBoundaryCondition(const std::string & bc_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  FEProblemBase::addUserObject(bc_name, name, parameters);
  const UserObject * mfem_bc_uo = &(getUserObjectBase(name));
  if (dynamic_cast<const MFEMIntegratedBC *>(mfem_bc_uo) != nullptr)
  {
    auto object_ptr = getUserObject<MFEMIntegratedBC>(name).getSharedPtr();
    auto bc = std::dynamic_pointer_cast<MFEMIntegratedBC>(object_ptr);
    bc->getBoundaries();
    if (getProblemData().eqn_system)
    {
      getProblemData().eqn_system->AddIntegratedBC(std::move(bc));
    }
    else
    {
      mooseError("Cannot add integrated BC with name '" + name +
                 "' because there is no corresponding equation system.");
    }
  }
  else if (dynamic_cast<const MFEMEssentialBC *>(mfem_bc_uo) != nullptr)
  {
    auto object_ptr = getUserObject<MFEMEssentialBC>(name).getSharedPtr();
    auto mfem_bc = std::dynamic_pointer_cast<MFEMEssentialBC>(object_ptr);
    mfem_bc->getBoundaries();
    if (getProblemData().eqn_system)
    {
      getProblemData().eqn_system->AddEssentialBC(std::move(mfem_bc));
    }
    else
    {
      mooseError("Cannot add boundary condition with name '" + name +
                 "' because there is no corresponding equation system.");
    }
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
  FEProblemBase::addUserObject(material_name, name, parameters);
  getUserObject<MFEMFunctorMaterial>(name);
}

void
MFEMProblem::addFESpace(const std::string & user_object_name,
                        const std::string & name,
                        InputParameters & parameters)
{
  FEProblemBase::addUserObject(user_object_name, name, parameters);
  MFEMFESpace & mfem_fespace(getUserObject<MFEMFESpace>(name));

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
    addGridFunction(var_type, Moose::MFEM::GetTimeDerivativeName(var_name), parameters);
  }
}

void
MFEMProblem::addGridFunction(const std::string & var_type,
                             const std::string & var_name,
                             InputParameters & parameters)
{
  if (var_type == "MFEMVariable")
  {
    // Add MFEM variable directly.
    FEProblemBase::addUserObject(var_type, var_name, parameters);
  }
  else
  {
    // Add MOOSE variable.
    FEProblemBase::addVariable(var_type, var_name, parameters);

    // Add MFEM variable indirectly ("gridfunction").
    InputParameters mfem_variable_params = addMFEMFESpaceFromMOOSEVariable(parameters);
    FEProblemBase::addUserObject("MFEMVariable", var_name, mfem_variable_params);
  }

  // Register gridfunction.
  MFEMVariable & mfem_variable = getUserObject<MFEMVariable>(var_name);
  getProblemData().gridfunctions.Register(var_name, mfem_variable.getGridFunction());
  if (mfem_variable.getFESpace().isScalar())
    getCoefficients().declareScalar<mfem::GridFunctionCoefficient>(
        var_name, mfem_variable.getGridFunction().get());
  else
    getCoefficients().declareVector<mfem::VectorGridFunctionCoefficient>(
        var_name, mfem_variable.getGridFunction().get());
}

void
MFEMProblem::addAuxVariable(const std::string & var_type,
                            const std::string & var_name,
                            InputParameters & parameters)
{
  // We do not handle MFEM AuxVariables separately from variables currently
  addVariable(var_type, var_name, parameters);
}

void
MFEMProblem::addAuxKernel(const std::string & kernel_name,
                          const std::string & name,
                          InputParameters & parameters)
{
  FEProblemBase::addUserObject(kernel_name, name, parameters);
}

void
MFEMProblem::addKernel(const std::string & kernel_name,
                       const std::string & name,
                       InputParameters & parameters)
{
  FEProblemBase::addUserObject(kernel_name, name, parameters);
  const UserObject * kernel_uo = &(getUserObjectBase(name));

  if (dynamic_cast<const MFEMKernel *>(kernel_uo) != nullptr)
  {
    auto object_ptr = getUserObject<MFEMKernel>(name).getSharedPtr();
    auto kernel = std::dynamic_pointer_cast<MFEMKernel>(object_ptr);
    if (getProblemData().eqn_system)
    {
      getProblemData().eqn_system->AddKernel(std::move(kernel));
    }
    else
    {
      mooseError("Cannot add kernel with name '" + name +
                 "' because there is no corresponding equation system.");
    }
  }
  else
  {
    mooseError("Unsupported kernel of type '", kernel_name, "' and name '", name, "' detected.");
  }
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
                                               "RichardsExcavGeom",
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
        [&func](const mfem::Vector & p, double t) -> mfem::real_t
        { return func.value(t, pointFromMFEMVector(p)); });
  }
  else if (std::find(VECTOR_FUNCS.begin(), VECTOR_FUNCS.end(), type) != VECTOR_FUNCS.end())
  {
    int dim = vectorFunctionDim(type, parameters);
    getCoefficients().declareVector<mfem::VectorFunctionCoefficient>(
        name,
        dim,
        [&func, dim](const mfem::Vector & p, double t, mfem::Vector & u)
        {
          libMesh::RealVectorValue vector_value = func.vectorValue(t, pointFromMFEMVector(p));
          for (int i = 0; i < dim; i++)
          {
            u[i] = vector_value(i);
          }
        });
  }
  else
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
  // For some reason this isn't getting called
  ExternalProblem::addPostprocessor(type, name, parameters);
  const PostprocessorValue & val = getPostprocessorValueByName(name);
  getCoefficients().declareScalar<mfem::FunctionCoefficient>(
      name, [&val](const mfem::Vector &, double) -> mfem::real_t { return val; });
}

InputParameters
MFEMProblem::addMFEMFESpaceFromMOOSEVariable(InputParameters & parameters)
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

  if (!hasUserObject(fespace_name)) // Create the fespace (implicit).
  {
    addFESpace("MFEMGenericFESpace", fespace_name, fespace_params);
  }

  mfem_variable_params.set<UserObjectName>("fespace") = fespace_name;

  return mfem_variable_params;
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

#endif
