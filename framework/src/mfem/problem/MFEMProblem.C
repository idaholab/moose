#include "MFEMProblem.h"

#include <vector>
#include <algorithm>

registerMooseObject("PlatypusApp", MFEMProblem);

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
  getProblemData()._pmesh = pmesh;
  getProblemData()._comm = pmesh->GetComm();
  MPI_Comm_size(pmesh->GetComm(), &(getProblemData()._num_procs));
  MPI_Comm_rank(pmesh->GetComm(), &(getProblemData()._myid));
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
  const MFEMSolverBase & mfem_preconditioner = getUserObject<MFEMSolverBase>(name);

  getProblemData()._jacobian_preconditioner = mfem_preconditioner.getSolver();
}

void
MFEMProblem::addMFEMSolver(const std::string & user_object_name,
                           const std::string & name,
                           InputParameters & parameters)
{
  FEProblemBase::addUserObject(user_object_name, name, parameters);
  const MFEMSolverBase & mfem_solver = getUserObject<MFEMSolverBase>(name);

  getProblemData()._jacobian_solver = mfem_solver.getSolver();
}

void
MFEMProblem::addMFEMNonlinearSolver()
{
  auto nl_solver = std::make_shared<mfem::NewtonSolver>(getProblemData()._comm);

  // Defaults to one iteration, without further nonlinear iterations
  nl_solver->SetRelTol(0.0);
  nl_solver->SetAbsTol(0.0);
  nl_solver->SetMaxIter(1);

  getProblemData()._nonlinear_solver = nl_solver;
}

void
MFEMProblem::addBoundaryCondition(const std::string & bc_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  FEProblemBase::addUserObject(bc_name, name, parameters);

  auto object_ptr = getUserObject<MFEMBoundaryCondition>(name).getSharedPtr();
  auto mfem_bc = std::dynamic_pointer_cast<MFEMBoundaryCondition>(object_ptr);

  if (getProblemData()._bc_map.Has(name))
  {
    const std::string error_message = "A boundary condition with the name " + name +
                                      " has already been added to the problem boundary conditions.";
    mfem::mfem_error(error_message.c_str());
  }
  getProblemData()._bc_map.Register(name, std::move(mfem_bc));
}

void
MFEMProblem::addMaterial(const std::string & kernel_name,
                         const std::string & name,
                         InputParameters & parameters)
{
  FEProblemBase::addUserObject(kernel_name, name, parameters);
  getUserObject<MFEMMaterial>(name);
}

void
MFEMProblem::addFESpace(const std::string & user_object_name,
                        const std::string & name,
                        InputParameters & parameters)
{
  FEProblemBase::addUserObject(user_object_name, name, parameters);
  MFEMFESpace & mfem_fespace(getUserObject<MFEMFESpace>(name));

  // Register fespace and associated fe collection.
  getProblemData()._fecs.Register(name, mfem_fespace.getFEC());
  getProblemData()._fespaces.Register(name, mfem_fespace.getFESpace());
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
    addGridFunction(var_type, platypus::GetTimeDerivativeName(var_name), parameters);
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
  getProblemData()._gridfunctions.Register(var_name, mfem_variable.getGridFunction());
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
  const UserObject * kernel = &(getUserObjectBase(name));

  if (dynamic_cast<const MFEMKernel<mfem::LinearFormIntegrator> *>(kernel) != nullptr)
  {
    auto object_ptr = getUserObject<MFEMKernel<mfem::LinearFormIntegrator>>(name).getSharedPtr();
    auto lf_kernel = std::dynamic_pointer_cast<MFEMKernel<mfem::LinearFormIntegrator>>(object_ptr);

    addKernel(lf_kernel->getTestVariableName(), lf_kernel);
  }
  else if (dynamic_cast<const MFEMMixedBilinearFormKernel *>(kernel) != nullptr)
  {
    auto object_ptr = getUserObject<MFEMMixedBilinearFormKernel>(name).getSharedPtr();
    auto mblf_kernel = std::dynamic_pointer_cast<MFEMMixedBilinearFormKernel>(object_ptr);
    addKernel(mblf_kernel->getTrialVariableName(), mblf_kernel->getTestVariableName(), mblf_kernel);
  }
  else if (dynamic_cast<const MFEMKernel<mfem::BilinearFormIntegrator> *>(kernel) != nullptr)
  {
    auto object_ptr = getUserObject<MFEMKernel<mfem::BilinearFormIntegrator>>(name).getSharedPtr();
    auto blf_kernel =
        std::dynamic_pointer_cast<MFEMKernel<mfem::BilinearFormIntegrator>>(object_ptr);
    addKernel(blf_kernel->getTestVariableName(), blf_kernel);
  }
  else
  {
    mooseError("Unsupported kernel of type '", kernel_name, "' and name '", name, "' detected.");
  }
}

/**
 * Method for adding mixed bilinear kernels. We can only add kernels using equation system problem
 * builders.
 */
void
MFEMProblem::addKernel(std::string trial_var_name,
                       std::string test_var_name,
                       std::shared_ptr<MFEMMixedBilinearFormKernel> kernel)
{
  using namespace platypus;
  if (getProblemData()._eqn_system)
  {
    getProblemData()._eqn_system->AddKernel(trial_var_name, test_var_name, std::move(kernel));
  }
  else
  {
    mooseError("Cannot add kernel with name '" + test_var_name +
               "' because there is no equation system.");
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
    _scalar_functions[name] = makeScalarCoefficient<mfem::FunctionCoefficient>(
        [&func](const mfem::Vector & p, double t) -> mfem::real_t
        { return func.value(t, pointFromMFEMVector(p)); });
  }
  else if (std::find(VECTOR_FUNCS.begin(), VECTOR_FUNCS.end(), type) != VECTOR_FUNCS.end())
  {
    int dim = vectorFunctionDim(type, parameters);
    _vector_functions[name] = makeVectorCoefficient<mfem::VectorFunctionCoefficient>(
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
    return *_problem_data._gridfunctions.Get(displacement_variable.value());
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

std::shared_ptr<mfem::FunctionCoefficient>
MFEMProblem::getScalarFunctionCoefficient(const std::string & name)
{
  try
  {
    return this->_scalar_functions.at(name);
  }
  catch (std::out_of_range)
  {
    mooseError("No scalar function with name '" + name + "'.");
  }
}

std::shared_ptr<mfem::VectorFunctionCoefficient>
MFEMProblem::getVectorFunctionCoefficient(const std::string & name)
{
  try
  {
    return this->_vector_functions.at(name);
  }
  catch (std::out_of_range)
  {
    mooseError("No vector function with name '" + name + "'.");
  }
}

MFEMMesh &
MFEMProblem::mesh()
{
  mooseAssert(ExternalProblem::mesh().type() == "MFEMMesh",
              "Please choose the MFEMMesh mesh type for an MFEMProblem\n");
  return (MFEMMesh &)_mesh;
}
