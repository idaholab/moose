/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "CreateProblemAction.h"
#include "Factory.h"
#include "FEProblem.h"
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
    }
    // set up the problem
    _problem->setCoordSystem(_blocks, _coord_sys);
    _problem->setAxisymmetricCoordAxis(getParam<MooseEnum>("rz_coord_axis"));
    _problem->useFECache(_fe_cache);
    _problem->setKernelCoverageCheck(getParam<bool>("kernel_coverage_check"));
    _problem->setMaterialCoverageCheck(getParam<bool>("material_coverage_check"));

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
        restart_file_base = MooseUtils::getRecoveryFileBase(files);

        if (restart_file_base == "")
          mooseError("Unable to find suitable restart file");
      }

      _console << "\nUsing " << restart_file_base << " for restart.\n\n";
      _problem->setRestartFile(restart_file_base);
    }
  }
}
