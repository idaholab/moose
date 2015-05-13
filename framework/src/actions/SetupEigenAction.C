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

#include "SetupEigenAction.h"
#include "EigenExecutionerBase.h"
#include "MooseObjectAction.h"

template<>
InputParameters validParams<SetupEigenAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

SetupEigenAction::SetupEigenAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters)
{
}

void
SetupEigenAction::act()
{
  MooseSharedPointer<EigenExecutionerBase> exec = MooseSharedNamespace::dynamic_pointer_cast<EigenExecutionerBase>(_executioner);




  std::vector<Action *> actions = _awh.getActionsByName("add_kernel");
  for (std::vector<Action *>::iterator it = actions.begin(); it != actions.end(); ++it)
  {
    MooseObjectAction * moa = dynamic_cast<MooseObjectAction *>(*it);
    if (moa)
    {
      InputParameters & params = moa->getObjectParams();


      if (params.have_parameter<std::string>("_moose_base") &&
          params.get<std::string>("_moose_base") == "EigenKernel")
      {

        if (!params.isParamValid("eigen_postprocessor"))
        {

          PostprocessorName pp_name = "1.0";
          if (exec)
            pp_name = exec->getParam<PostprocessorName>("bx_norm");
          else if (!_problem->hasPostprocessor(pp_name))
            _problem->addPostprocessor("EigenValueReporter", pp_name, _factory.getValidParams("EigenValueReporter"));


          params.set<PostprocessorName>("eigen_postprocessor") = pp_name;
          std::cout << "eigen_pp = " << pp_name << std::endl;
        }
      }
    }
  }
}
