#pragma once
#include <vector>

#include "MultiAppFieldTransfer.h"
#include "MultiMooseEnum.h"
#include "MultiApp.h"
#include "MooseAppCoordTransform.h"
#include "MFEMMesh.h"

#include "libmesh/bounding_box.h"

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
    
	//std::vector<VariableName> getFromVarNames() const override {return std::vector<VariableName>();};
    //std::vector<AuxVariableName> getToVarNames() const override {return std::vector<AuxVariableName>();}
	
	//std::vector<FEProblemBase*> _to_problems;
	//std::vector<FEProblemBase*> _from_problems;
	//std::vector<MooseMesh *> _to_meshes;
	//std::vector<MooseMesh *> _from_meshes;
};
