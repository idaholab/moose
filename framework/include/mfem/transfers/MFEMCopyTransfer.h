#pragma once
#include <vector>

#include "MultiAppTransfer.h"
#include "MultiApp.h"
#include "MooseAppCoordTransform.h"
#include "MFEMProblem.h"

class MooseMesh;

class MFEMCopyTransfer : public MultiAppTransfer
{
public:
	static InputParameters validParams();           
	MFEMCopyTransfer(InputParameters const & params);
	//void initialSetup() override;
	//virtual getAppInfo();
	void execute() override;
    void checkSiblingsTransferSupported() const override;
    auto const & getFromVarName(int i) {return _from_var_names.at(i);};
    auto const & getToVarName(int i) {return _to_var_names.at(i);};
	auto numFromVar() {return _from_var_names.size();}
	auto numToVar() {return _to_var_names.size();}
protected:
	std::vector<VariableName> _from_var_names;
	//std::vector<MFEMVariable*> _from_vars;
	std::vector<AuxVariableName> _to_var_names;
	//std::vector<MFEMVariable*> _to_vars;
	void transfer(MFEMProblem &to_problem, MFEMProblem &from_problem);
};
