//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateProblemAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "EigenProblem.h"
#include "NonlinearSystemBase.h"
#include "MooseApp.h"
#include "CreateExecutionerAction.h"

registerMooseAction("MooseApp", CreateProblemAction, "meta_action");
registerMooseAction("MooseApp", CreateProblemAction, "create_problem");

template <>
InputParameters
validParams<CreateProblemAction>()
{
  MultiMooseEnum coord_types("XYZ RZ RSPHERICAL", "XYZ");
  MooseEnum rz_coord_axis("X=0 Y=1", "Y");

  InputParameters params = validParams<Action>();
  params.addParam<std::string>("type", "The type of the problem");
  params.addParam<std::string>("name", "MOOSE Problem", "The name the problem");
  params.addParam<std::vector<SubdomainName>>("block", "Block IDs for the coordinate systems");
  params.addParam<MultiMooseEnum>(
      "coord_type", coord_types, "Type of the coordinate system per block param");
  params.addParam<MooseEnum>(
      "rz_coord_axis", rz_coord_axis, "The rotation axis (X | Y) for axisymetric coordinates");
  params.addParam<bool>(
      "kernel_coverage_check", true, "Set to false to disable kernel->subdomain coverage check");
  params.addParam<bool>("material_coverage_check",
                        true,
                        "Set to false to disable material->subdomain coverage check");
  params.addParam<bool>("parallel_barrier_messaging",
                        true,
                        "Displays messaging from parallel "
                        "barrier notifications when executing "
                        "or transferring to/from Multiapps "
                        "(default: true)");

  // Set this private parameter to true to skip setting problem parameters from input and solving
  // the problem
  params.addPrivateParam<bool>("_skip_param_construction_and_solving");

  params.addParam<FileNameNoExtension>("restart_file_base",
                                       "File base name used for restart (e.g. "
                                       "<path>/<filebase> or <path>/LATEST to "
                                       "grab the latest file available)");

  params.addParam<std::vector<TagName>>("extra_tag_vectors",
                                        "Extra vectors to add to the system that can be filled by "
                                        "objects which compute residuals and Jacobians (Kernels, "
                                        "BCs, etc.) by setting tags on them.");

  params.addParam<std::vector<TagName>>("extra_tag_matrices",
                                        "Extra matrices to add to the system that can be filled "
                                        "by objects which compute residuals and Jacobians "
                                        "(Kernels, BCs, etc.) by setting tags on them.");

  return params;
}

CreateProblemAction::CreateProblemAction(InputParameters parameters)
  : Action(parameters),
    _blocks(getParam<std::vector<SubdomainName>>("block")),
    _coord_sys(getParam<MultiMooseEnum>("coord_type"))
{
}

void
CreateProblemAction::act()
{
  bool use_nonlinear = true;
  bool use_eigenvalue = false;
  if (_awh.hasActions("setup_executioner"))
  {
    CreateExecutionerAction * p = nullptr;
    const auto & actions = _awh.getActionListByName("setup_executioner");
    for (const auto & action : actions)
    {
      p = dynamic_cast<CreateExecutionerAction *>(action);
      if (p)
        break;
    }
    if (p)
    {
      auto & exparams = p->getObjectParams();
      use_nonlinear = !(exparams.isParamValid("_eigen") && exparams.get<bool>("_eigen"));
      use_eigenvalue =
          (exparams.isParamValid("_use_eigen_value") && exparams.get<bool>("_use_eigen_value"));
    }
  }

  if (_current_task == "meta_action")
  {
    // MOOSE sets the default problem type on meta_action task, which allows other actions
    // to reset problem type later through MooseApp::problemType().
    if (!isParamValid("type"))
    {
      if (use_eigenvalue)
        _app.problemType() = "EigenProblem";
      else
        _app.problemType() = "FEProblem";
    }
    else
      _app.problemType() = getParam<std::string>("type");
    return;
  }

  // build the problem only if we have mesh
  if (_mesh.get() != NULL)
  {
    // If users set type parameter in the input or the parameter is made valid by other actions,
    // use it as the final problem type.
    if (isParamValid("type"))
      _app.problemType() = getParam<std::string>("type");

    auto & type = _app.problemType();
    InputParameters params = _factory.getValidParams(type);

    if (_pars.isParamValid("_skip_param_construction_and_solving") &&
        _pars.get<bool>("_skip_param_construction_and_solving"))
      params.set<bool>("solve") = false;
    else
    {
      // retrieve extra parameters from Problem input block
      params.blockLocation() = _pars.blockLocation();
      params.blockFullpath() = _pars.blockFullpath();
      if (!params.blockFullpath().empty())
        _app.parser().extractParams(_pars.blockFullpath(), params);
      params.set<std::vector<std::string>>("control_tags")
          .push_back(MooseUtils::baseName(_pars.blockFullpath()));
    }

    params.set<MooseMesh *>("mesh") = _mesh.get();
    params.set<bool>("use_nonlinear") = use_nonlinear;

    _problem = _factory.create<FEProblemBase>(type, getParam<std::string>("name"), params);
    if (!_problem.get())
      mooseError("Problem has to be of a FEProblemBase type");

    // the type has to be an EigenProblem or its derived subclass when uing an eigen executioner
    if (use_eigenvalue && type != "EigenProblem" &&
        !(std::dynamic_pointer_cast<EigenProblem>(_problem)))
      mooseError("Problem has to be of a EigenProblem (or derived subclass) type when using "
                 "eigen executioner");

    // set up the problem
    _problem->setCoordSystem(_blocks, _coord_sys);
    _problem->setAxisymmetricCoordAxis(getParam<MooseEnum>("rz_coord_axis"));
    _problem->setKernelCoverageCheck(getParam<bool>("kernel_coverage_check"));
    _problem->setMaterialCoverageCheck(getParam<bool>("material_coverage_check"));
    _problem->setParallelBarrierMessaging(getParam<bool>("parallel_barrier_messaging"));

    if (isParamValid("restart_file_base"))
    {
      std::string restart_file_base = getParam<FileNameNoExtension>("restart_file_base");

      std::size_t slash_pos = restart_file_base.find_last_of("/");
      std::string path = restart_file_base.substr(0, slash_pos);
      std::string file = restart_file_base.substr(slash_pos + 1);

      /**
       * If the user specified LATEST as the file in their directory path, find the file with the
       * latest timestep and the largest serial number.
       */
      if (file == "LATEST")
      {
        std::list<std::string> dir_list(1, path);
        std::list<std::string> files = MooseUtils::getFilesInDirs(dir_list);
        restart_file_base = MooseUtils::getLatestAppCheckpointFileBase(files);

        if (restart_file_base == "")
          mooseError("Unable to find suitable restart file");
      }

      _console << "\nUsing " << restart_file_base << " for restart.\n\n";
      _problem->setRestartFile(restart_file_base);
    }

    // Create etra vectors and matrices if any
    CreateTagVectors();
  }
}

void
CreateProblemAction::CreateTagVectors()
{
  // add vectors and their tags to system
  auto & vectors = getParam<std::vector<TagName>>("extra_tag_vectors");
  auto & nl = _problem->getNonlinearSystemBase();
  for (auto & vector : vectors)
  {
    auto tag = _problem->addVectorTag(vector);
    nl.addVector(tag, false, GHOSTED);
  }

  // add matrices and their tags
  auto & matrices = getParam<std::vector<TagName>>("extra_tag_matrices");
  for (auto & matrix : matrices)
  {
    auto tag = _problem->addMatrixTag(matrix);
    nl.addMatrix(tag);
  }
}
