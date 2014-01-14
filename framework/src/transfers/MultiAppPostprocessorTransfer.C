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

#include "MultiAppPostprocessorTransfer.h"

// Moose
#include "MooseTypes.h"
#include "FEProblem.h"

// libMesh
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

template<>
InputParameters validParams<MultiAppPostprocessorTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<PostprocessorName>("from_postprocessor", "The name of the Postprocessor in the Master to transfer the value from.");
  params.addRequiredParam<PostprocessorName>("to_postprocessor", "The name of the Postprocessor in the MultiApp to transfer the value to.  This should most likely be a Reporter Postprocessor.");
  return params;
}

MultiAppPostprocessorTransfer::MultiAppPostprocessorTransfer(const std::string & name, InputParameters parameters) :
    MultiAppTransfer(name, parameters),
    _from_pp_name(getParam<PostprocessorName>("from_postprocessor")),
    _to_pp_name(getParam<PostprocessorName>("to_postprocessor"))
{
}

void
MultiAppPostprocessorTransfer::execute()
{
  switch(_direction)
  {
    case TO_MULTIAPP:
    {
      FEProblem & from_problem = *_multi_app->problem();

      Real pp_value = from_problem.getPostprocessorValue(_from_pp_name);

      for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
        if(_multi_app->hasLocalApp(i))
          _multi_app->appProblem(i)->getPostprocessorValue(_to_pp_name) = pp_value;

      break;
    }
    case FROM_MULTIAPP:
    {
      mooseError("Can't transfer a Postprocessor from a MultiApp to a Postprocessor in the Master!  This doesn't make sense because you would have multiple Postprocessors from each Sub-App feeding into just one Postprocessor in the Master!");
      break;
    }
  }
}
