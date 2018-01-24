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
#include "MooseApp.h"

template <>
InputParameters
validParams<CreateProblemAction>()
{
  MultiMooseEnum coord_types("XYZ RZ RSPHERICAL", "XYZ");
  MooseEnum rz_coord_axis("X=0 Y=1", "Y");

  InputParameters params = validParams<MooseObjectAction>();
  params.set<std::string>("type") = "FEProblem";
  params.addParam<std::string>("name", "MOOSE Problem", "The name the problem");
  params.addParam<std::vector<SubdomainName>>("block", "Block IDs for the coordinate systems");
  params.addParam<MultiMooseEnum>(
      "coord_type", coord_types, "Type of the coordinate system per block param");
  params.addParam<MooseEnum>(
      "rz_coord_axis", rz_coord_axis, "The rotation axis (X | Y) for axisymetric coordinates");

  params.addParam<bool>("fe_cache",
                        false,
                        "Whether or not to turn on the finite element shape "
                        "function caching system.  This can increase speed with "
                        "an associated memory cost.");

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

  params.addParam<FileNameNoExtension>("restart_file_base",
                                       "File base name used for restart (e.g. "
                                       "<path>/<filebase> or <path>/LATEST to "
                                       "grab the latest file available)");

  return params;
}

CreateProblemAction::CreateProblemAction(InputParameters parameters)
  : MooseObjectAction(parameters),
    _blocks(getParam<std::vector<SubdomainName>>("block")),
    _coord_sys(getParam<MultiMooseEnum>("coord_type")),
    _fe_cache(getParam<bool>("fe_cache"))
{
}

void
CreateProblemAction::act()
{
  if (_mesh.get() != NULL)
  {
    // build the problem only if we have mesh
    {
      _moose_object_pars.set<MooseMesh *>("mesh") = _mesh.get();
      _moose_object_pars.set<bool>("use_nonlinear") = _app.useNonlinear();

#ifdef LIBMESH_HAVE_PETSC
      // put in empty arrays for PETSc options
      _moose_object_pars.set<MultiMooseEnum>("petsc_options") = MultiMooseEnum("", "", true);
      _moose_object_pars.set<std::vector<std::string>>("petsc_inames") = std::vector<std::string>();
      _moose_object_pars.set<std::vector<std::string>>("petsc_values") = std::vector<std::string>();
#endif
      _problem =
          _factory.create<FEProblemBase>(_type, getParam<std::string>("name"), _moose_object_pars);
      if (!_problem.get())
        mooseError("Problem has to be of a FEProblemBase type");

      // if users provide a problem type, the type has to be an EigenProblem or its derived subclass
      // when uing an eigen executioner
      if (_app.useEigenvalue() && _type != "EigenProblem" &&
          !(std::dynamic_pointer_cast<EigenProblem>(_problem)))
        mooseError("Problem has to be of a EigenProblem (or derived subclass) type when using "
                   "eigen executioner");
    }
    // set up the problem
    _problem->setCoordSystem(_blocks, _coord_sys);
    _problem->setAxisymmetricCoordAxis(getParam<MooseEnum>("rz_coord_axis"));
    _problem->useFECache(_fe_cache);
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
  }
}
