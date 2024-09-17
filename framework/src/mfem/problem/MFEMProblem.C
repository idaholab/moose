#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMProblem);

InputParameters
MFEMProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addParam<std::string>("device", "cpu", "Run app on the chosen device.");
  return params;
}

MFEMProblem::MFEMProblem(const InputParameters & params) : ExternalProblem(params), _coefficients()
{
}

void
MFEMProblem::outputStep(ExecFlagType type)
{
  // Needed to ensure outputs from successive runs when using MultiApps are stored in
  // directories with iterated names
  if (type == EXEC_INITIAL)
  {
    mfem_problem->_outputs.Init(mfem_problem->_gridfunctions);
    std::vector<OutputName> mfem_data_collections =
        _app.getOutputWarehouse().getOutputNames<MFEMDataCollection>();
    for (const auto & name : mfem_data_collections)
    {
      auto dc = _app.getOutputWarehouse().getOutput<MFEMDataCollection>(name);
      int filenum(dc->getFileNumber());
      std::string filename("/Run" + std::to_string(filenum));

      mfem_problem->_outputs.Register(name, dc->createDataCollection(filename));
      mfem_problem->_outputs.Reset();
      dc->setFileNumber(filenum + 1);
    }
  }
  FEProblemBase::outputStep(type);
}

void
MFEMProblem::initialSetup()
{
  FEProblemBase::initialSetup();
  _coefficients.AddGlobalCoefficientsFromSubdomains();

  getProblemData()._coefficients = _coefficients;
  mfem_problem_builder->FinalizeProblem(false);

  if (dynamic_cast<MFEMExecutioner *>(_app.getExecutioner()) == nullptr)
  {
    mooseError("Executioner used that is not currently supported by MFEMProblem");
  }
}

void
MFEMProblem::init()
{
  FEProblemBase::init();
}

void
MFEMProblem::setDevice()
{
  mfem_problem->_device.Configure(getParam<std::string>("device"));
  mfem_problem->_device.Print(std::cout);
}

void
MFEMProblem::setMesh(std::shared_ptr<mfem::ParMesh> pmesh)
{
  mfem_problem->_pmesh = pmesh;
  mfem_problem->_comm = pmesh->GetComm();
  MPI_Comm_size(pmesh->GetComm(), &(mfem_problem->_num_procs));
  MPI_Comm_rank(pmesh->GetComm(), &(mfem_problem->_myid));
}

void
MFEMProblem::setProblemBuilder()
{
  mfem::ParMesh & mfem_par_mesh = mesh().getMFEMParMesh();
  if (isTransient())
  {
    mfem_problem_builder = std::make_shared<platypus::TimeDomainEquationSystemProblemBuilder>();
  }
  else
  {
    mfem_problem_builder = std::make_shared<platypus::SteadyStateEquationSystemProblemBuilder>();
  }
  mfem_problem = mfem_problem_builder->ReturnProblem();

  setDevice();
  setMesh(std::make_shared<mfem::ParMesh>(mfem_par_mesh));
  mfem_problem->ConstructOperator();
}

void
MFEMProblem::addMFEMPreconditioner(const std::string & user_object_name,
                                   const std::string & name,
                                   InputParameters & parameters)
{
  FEProblemBase::addUserObject(user_object_name, name, parameters);
  const MFEMSolverBase & mfem_preconditioner = getUserObject<MFEMSolverBase>(name);

  mfem_problem->_jacobian_preconditioner = mfem_preconditioner.getSolver();
}

void
MFEMProblem::addMFEMSolver(const std::string & user_object_name,
                           const std::string & name,
                           InputParameters & parameters)
{
  FEProblemBase::addUserObject(user_object_name, name, parameters);
  const MFEMSolverBase & mfem_solver = getUserObject<MFEMSolverBase>(name);

  mfem_problem->_jacobian_solver = mfem_solver.getSolver();
}

void
MFEMProblem::addBoundaryCondition(const std::string & bc_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  FEProblemBase::addUserObject(bc_name, name, parameters);
  MFEMBoundaryCondition * mfem_bc(&getUserObject<MFEMBoundaryCondition>(name));

  if (mfem_problem->_bc_map.Has(name))
  {
    const std::string error_message = "A boundary condition with the name " + name +
                                      " has already been added to the problem boundary conditions.";
    mfem::mfem_error(error_message.c_str());
  }
  mfem_problem->_bc_map.Register(name, std::move(mfem_bc->getBC()));
}

void
MFEMProblem::addMaterial(const std::string & kernel_name,
                         const std::string & name,
                         InputParameters & parameters)
{
  FEProblemBase::addUserObject(kernel_name, name, parameters);
  MFEMMaterial & mfem_material(getUserObject<MFEMMaterial>(name));
}

void
MFEMProblem::addCoefficient(const std::string & user_object_name,
                            const std::string & name,
                            InputParameters & parameters)
{
  FEProblemBase::addUserObject(user_object_name, name, parameters);
  MFEMCoefficient * mfem_coef(&getUserObject<MFEMCoefficient>(name));
  _coefficients._scalars.Register(name, mfem_coef->getCoefficient());
}

void
MFEMProblem::addVectorCoefficient(const std::string & user_object_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  FEProblemBase::addUserObject(user_object_name, name, parameters);
  MFEMVectorCoefficient * mfem_vec_coef(&getUserObject<MFEMVectorCoefficient>(name));
  _coefficients._vectors.Register(name, mfem_vec_coef->getVectorCoefficient());
}

void
MFEMProblem::addFESpace(const std::string & user_object_name,
                        const std::string & name,
                        InputParameters & parameters)
{
  FEProblemBase::addUserObject(user_object_name, name, parameters);
  MFEMFESpace & mfem_fespace(getUserObject<MFEMFESpace>(name));

  // Register fespace and associated fe collection.
  mfem_problem->_fecs.Register(name, mfem_fespace.getFEC());
  mfem_problem->_fespaces.Register(name, mfem_fespace.getFESpace());
}

void
MFEMProblem::addVariable(const std::string & var_type,
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
    // Add MOOSE auxvariable.
    FEProblemBase::addVariable(var_type, var_name, parameters);

    // Add MFEM variable indirectly ("gridfunction").
    InputParameters mfem_variable_params = addMFEMFESpaceFromMOOSEVariable(parameters);
    FEProblemBase::addUserObject("MFEMVariable", var_name, mfem_variable_params);
  }

  // Register gridfunction.
  MFEMVariable & mfem_variable = getUserObject<MFEMVariable>(var_name);
  mfem_problem->_gridfunctions.Register(var_name, mfem_variable.getGridFunction());
}

void
MFEMProblem::addAuxVariable(const std::string & var_type,
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
    // Add MOOSE auxvariable.
    FEProblemBase::addAuxVariable(var_type, var_name, parameters);

    // Add MFEM variable indirectly ("gridfunction").
    InputParameters mfem_variable_params = addMFEMFESpaceFromMOOSEVariable(parameters);
    FEProblemBase::addUserObject("MFEMVariable", var_name, mfem_variable_params);
  }

  // Register gridfunction.
  MFEMVariable & mfem_variable = getUserObject<MFEMVariable>(var_name);
  mfem_problem->_gridfunctions.Register(var_name, mfem_variable.getGridFunction());
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

    addKernel(parameters.get<std::string>("variable"), lf_kernel);
  }
  else if (dynamic_cast<const MFEMKernel<mfem::BilinearFormIntegrator> *>(kernel) != nullptr)
  {
    auto object_ptr = getUserObject<MFEMKernel<mfem::BilinearFormIntegrator>>(name).getSharedPtr();
    auto blf_kernel =
        std::dynamic_pointer_cast<MFEMKernel<mfem::BilinearFormIntegrator>>(object_ptr);
    addKernel(parameters.get<std::string>("variable"), blf_kernel);
  }
  else
  {
    mooseError("Unsupported kernel of type '", kernel_name, "' and name '", name, "' detected.");
  }
}

InputParameters
MFEMProblem::addMFEMFESpaceFromMOOSEVariable(InputParameters & parameters)
{
  InputParameters fespace_params = _factory.getValidParams("MFEMFESpace");
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

  // Set all fespace parameters.
  fespace_params.set<MooseEnum>("fec_order") = moose_fe_type.order;
  fespace_params.set<MooseEnum>("fec_type") = mfem_family;
  fespace_params.set<int>("vdim") = mfem_vdim;

  // Create unique fespace name. If this already exists, we will reuse this for
  // the mfem variable ("gridfunction").
  const std::string fespace_name =
      mfem_family + "_3D_P" + std::to_string(moose_fe_type.order.get_order());

  if (!hasUserObject(fespace_name)) // Create the fespace (implicit).
  {
    addFESpace("MFEMFESpace", fespace_name, fespace_params);
  }

  mfem_variable_params.set<UserObjectName>("fespace") = fespace_name;

  return mfem_variable_params;
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
  return (MFEMMesh &)_mesh;
}
