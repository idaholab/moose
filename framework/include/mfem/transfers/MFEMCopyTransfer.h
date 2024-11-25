#pragma once
#include <vector>

#include "MultiAppTransfer.h"
#include "MultiApp.h"
#include "MooseAppCoordTransform.h"

class MooseMesh;

class MFEMCopyTransfer : public MultiAppTransfer
{
public:
	static InputParameters validParams();           
	MFEMCopyTransfer(InputParameters const & params);
	//void initialSetup() override;
	//virtual getAppInfo();
	void execute() override;
protected:
	std::vector<VariableName> _from_var_names;
	std::vector<AuxVariableName> _to_var_names;
};
