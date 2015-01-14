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

template<>
InputParameters validParams<CreateProblemAction>()
{
  MultiMooseEnum coord_types("XYZ RZ RSPHERICAL", "XYZ");
  MooseEnum rz_coord_axis("X=0 Y=1", "Y");

  InputParameters params = validParams<MooseObjectAction>();
  params.set<std::string>("type") = "FEProblem";
  params.addParam<std::string>("name", "MOOSE Problem", "The name the problem");
  params.addParam<std::vector<SubdomainName> >("block", "Block IDs for the coordinate systems");
  params.addParam<MultiMooseEnum>("coord_type", coord_types, "Type of the coordinate system per block param");
  params.addParam<MooseEnum>("rz_coord_axis", rz_coord_axis, "The rotation axis (X | Y) for axisymetric coordinates");

  params.addParam<bool>("fe_cache", false, "Whether or not to turn on the finite element shape function caching system.  This can increase speed with an associated memory cost.");

  params.addParam<bool>("kernel_coverage_check", true, "Set to false to disable kernel->subdomain kernel coverage check");

  params.addParam<bool>("use_legacy_uo_aux_computation", "Set to true to have MOOSE recompute *all* AuxKernel types every time *any* UserObject type is executed.\nThis behavoir is non-intuitive and will be removed late fall 2014, The default is controlled through MooseApp");
  params.addParam<bool>("use_legacy_uo_initialization", "Set to true to have MOOSE compute all UserObjects and Postprocessors during the initial setup phase of the problem recompute *all* AuxKernel types every time *any* UserObject type is executed.\nThis behavoir is non-intuitive and will be removed late fall 2014, The default is controlled through MooseApp");


  return params;
}


CreateProblemAction::CreateProblemAction(const std::string & name, InputParameters parameters) :
    MooseObjectAction(name, parameters),
    _problem_name(getParam<std::string>("name")),
    _blocks(getParam<std::vector<SubdomainName> >("block")),
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
      _problem = MooseSharedNamespace::dynamic_pointer_cast<FEProblem>(_factory.create(_type, _problem_name, _moose_object_pars));
      if (!_problem.get())
        mooseError("Problem has to be of a FEProblem type");
    }
    // set up the problem
    _problem->setCoordSystem(_blocks, _coord_sys);
    _problem->setAxisymmetricCoordAxis(getParam<MooseEnum>("rz_coord_axis"));
    _problem->useFECache(_fe_cache);
    _problem->setKernelCoverageCheck(getParam<bool>("kernel_coverage_check"));
    _problem->legacyUoAuxComputation() = _pars.isParamValid("use_legacy_uo_aux_computation") ? getParam<bool>("use_legacy_uo_aux_computation") : _app.legacyUoAuxComputationDefault();
    _problem->legacyUoInitialization() = _pars.isParamValid("use_legacy_uo_initialization") ? getParam<bool>("use_legacy_uo_initialization") : _app.legacyUoInitializationDefault();
  }
}
