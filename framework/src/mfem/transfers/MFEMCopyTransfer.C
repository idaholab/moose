#include "MFEMCopyTransfer.h"
#include "FEProblemBase.h"
#include "MultiApp.h"
#include "SystemBase.h"
#include <memory>
#include "MFEMProblem.h"
#include "MFEMMesh.h"

registerMooseObject("PlatypusApp", MFEMCopyTransfer);

InputParameters
MFEMCopyTransfer::validParams()
{
	InputParameters params = MultiAppTransfer::validParams(); 
	params.addRequiredParam<std::vector<AuxVariableName>>(
		"variable", "AuxVariable to store transferred value in.");
	params.addRequiredParam<std::vector<VariableName>>(
		"source_variable", "Variable to transfer from");
	return params;
}

MFEMCopyTransfer::MFEMCopyTransfer(InputParameters const & params)
	: MultiAppTransfer(params),
	_from_var_names(getParam<std::vector<VariableName>>("source_variable")),
	_to_var_names(getParam<std::vector<AuxVariableName>>("variable")) 
{
}

void
MFEMCopyTransfer::execute()
{
  TIME_SECTION("MFEMCopyTransfer::execute", 5, "Copies variables");
  if (_current_direction == TO_MULTIAPP)
  {
    auto & from_data = static_cast<MFEMProblem &>(getToMultiApp()->problemBase()).getProblemData();
    // TODO: temp restriction? for debugging
    assert(_from_var_names.size() == 1);
    assert(_to_var_names.size() == 1);
    auto & from_var = from_data._gridfunctions.GetRef(_from_var_names[0]);
    for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
    {
      if (getToMultiApp()->hasLocalApp(i))
      {
        auto & to_data =
            static_cast<MFEMProblem &>(getToMultiApp()->appProblemBase(i)).getProblemData();
        // TODO: Not sure if aborting if can't find name is always
        // the correct behaviour - maybe it is here?
        // Don't want to Has() first seems wastful
        auto & to_var = to_data._gridfunctions.GetRef(_to_var_names[0]);
        to_var = from_var;
      }
    }
  }
  else if (_current_direction == FROM_MULTIAPP)
  {	
    auto & to_data = static_cast<MFEMProblem &>(getFromMultiApp()->problemBase()).getProblemData();
    // TODO: temp restriction? for debugging
    assert(_from_var_names.size() == 1);
    assert(_to_var_names.size() == 1);
    auto & to_var = to_data._gridfunctions.GetRef(_to_var_names[0]);
    for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
    {
      if (getFromMultiApp()->hasLocalApp(i))
      {
        auto & from_data =
            static_cast<MFEMProblem &>(getFromMultiApp()->appProblemBase(i)).getProblemData();
        // TODO: Not sure if aborting if can't find name is always
        // the correct behaviour - maybe it is here?
        // Don't want to Has() first seems wastful
        auto & from_var = from_data._gridfunctions.GetRef(_from_var_names[0]);
        to_var = from_var;
      }
    }
  }
  else if (_current_direction == BETWEEN_MULTIAPP)
  {
    printf("BETWEEN\n");
    //	if (!transfers_done)
    //    mooseError("BETWEEN_MULTIAPP transfer not supported if there is not at least one subapp "
    //             "per multiapp involved on each rank");
  }
}


