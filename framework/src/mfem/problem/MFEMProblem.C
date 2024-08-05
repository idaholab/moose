#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMProblem);

InputParameters
MFEMProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addParam<int>(
      "vis_steps",
      1,
      "Number of timesteps between successive write outs of data collections to file.");
  params.addParam<bool>(
      "use_glvis", false, "Attempt to open GLVis ports to display variables during simulation");
  params.addParam<std::string>("device", "cpu", "Run app on the chosen device.");

  return params;
}

MFEMProblem::MFEMProblem(const InputParameters & params)
  : ExternalProblem(params),
    _input_mesh(_mesh.parameters().get<MeshFileName>("file")),
    _coefficients(),
    _outputs(),
    _exec_params()
{
}

MFEMProblem::~MFEMProblem() {}

void
MFEMProblem::outputStep(ExecFlagType type)
{
  // Needed to ensure outputs from successive runs when using MultiApps are stored in
  // directories with iterated names
  if (type == EXEC_INITIAL)
  {
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
  EquationSystems & es = FEProblemBase::es();
  _solver_options.SetParam("Tolerance", float(es.parameters.get<Real>("linear solver tolerance")));
  _solver_options.SetParam("AbsTolerance",
                           float(es.parameters.get<Real>("linear solver absolute tolerance")));
  _solver_options.SetParam("MaxIter",
                           es.parameters.get<unsigned int>("linear solver maximum iterations"));
  _coefficients.AddGlobalCoefficientsFromSubdomains();

  mfem_problem_builder->SetCoefficients(_coefficients);
  mfem_problem_builder->SetSolverOptions(_solver_options);

  // NB: set to false to avoid reconstructing problem operator.
  mfem_problem_builder->FinalizeProblem(false);

  platypus::InputParameters exec_params;

  Transient * _moose_executioner = dynamic_cast<Transient *>(_app.getExecutioner());
  if (_moose_executioner != nullptr)
  {
    auto mfem_transient_problem_builder =
        std::dynamic_pointer_cast<platypus::TimeDomainProblemBuilder>(mfem_problem_builder);
    if (mfem_transient_problem_builder == nullptr)
    {
      mooseError("Specified formulation does not support Transient executioners");
    }

    exec_params.SetParam("StartTime", float(_moose_executioner->getStartTime()));
    exec_params.SetParam("TimeStep", float(dt()));
    exec_params.SetParam("EndTime", float(_moose_executioner->endTime()));
    exec_params.SetParam("VisualisationSteps", getParam<int>("vis_steps"));
    exec_params.SetParam("Problem", static_cast<platypus::TimeDomainProblem *>(mfem_problem.get()));

    executioner = std::make_unique<platypus::TransientExecutioner>(exec_params);
  }
  else if (dynamic_cast<Steady *>(_app.getExecutioner()))
  {
    auto mfem_steady_problem_builder =
        std::dynamic_pointer_cast<platypus::SteadyStateProblemBuilder>(mfem_problem_builder);
    if (mfem_steady_problem_builder == nullptr)
    {
      mooseError("Specified formulation does not support Steady executioners");
    }

    exec_params.SetParam("Problem",
                         static_cast<platypus::SteadyStateProblem *>(mfem_problem.get()));

    executioner = std::make_unique<platypus::SteadyExecutioner>(exec_params);
  }
  else
  {
    mooseError("Executioner used that is not currently supported by MFEMProblem");
  }

  mfem_problem->_outputs.EnableGLVis(getParam<bool>("use_glvis"));
}

void
MFEMProblem::init()
{
  FEProblemBase::init();
}

void
MFEMProblem::externalSolve()
{
  if (!_solve)
  {
    return;
  }

  auto * transient_mfem_exec = dynamic_cast<platypus::TransientExecutioner *>(executioner.get());
  if (transient_mfem_exec != nullptr)
  {
    transient_mfem_exec->_t_step = dt();
  }
  executioner->Solve();
}

void
MFEMProblem::setFormulation(const std::string & user_object_name,
                            const std::string & name,
                            InputParameters & parameters)
{
  mfem::ParMesh & mfem_par_mesh = mesh().getMFEMParMesh();
  FEProblemBase::addUserObject(user_object_name, name, parameters);
  MFEMFormulation * mfem_formulation(&getUserObject<MFEMFormulation>(name));

  mfem_problem_builder = mfem_formulation->getProblemBuilder();

  mfem_problem_builder->SetDevice(getParam<std::string>("device"));
  mfem_problem_builder->SetMesh(std::make_shared<mfem::ParMesh>(mfem_par_mesh));
  mfem_problem_builder->ConstructOperator();

  mfem_problem = mfem_problem_builder->ReturnProblem();
}

void
MFEMProblem::addBoundaryCondition(const std::string & bc_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  FEProblemBase::addUserObject(bc_name, name, parameters);
  MFEMBoundaryCondition * mfem_bc(&getUserObject<MFEMBoundaryCondition>(name));
  mfem_problem_builder->AddBoundaryCondition(name, mfem_bc->getBC());
}

void
MFEMProblem::addMaterial(const std::string & kernel_name,
                         const std::string & name,
                         InputParameters & parameters)
{
  FEProblemBase::addUserObject(kernel_name, name, parameters);
  MFEMMaterial & mfem_material(getUserObject<MFEMMaterial>(name));

  for (unsigned int bid = 0; bid < mfem_material.blocks.size(); ++bid)
  {
    int block = std::stoi(mfem_material.blocks[bid]);
    platypus::Subdomain mfem_subdomain(name, block);
    mfem_material.storeCoefficients(mfem_subdomain);
    _coefficients._subdomains.push_back(mfem_subdomain);
  }
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

  if (dynamic_cast<const MFEMLinearFormKernel *>(kernel) != nullptr)
  {
    MFEMLinearFormKernel * lf_kernel(&getUserObject<MFEMLinearFormKernel>(name));
    addKernel<mfem::ParLinearForm>(parameters.get<std::string>("variable"), lf_kernel->getKernel());
  }
  else if (dynamic_cast<const MFEMBilinearFormKernel *>(kernel) != nullptr)
  {
    MFEMBilinearFormKernel * blf_kernel(&getUserObject<MFEMBilinearFormKernel>(name));
    addKernel<mfem::ParBilinearForm>(parameters.get<std::string>("variable"),
                                     blf_kernel->getKernel());
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
