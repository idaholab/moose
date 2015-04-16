#ifndef RECONVARICACTION_H
#define RECONVARICACTION_H

#include "InputParameters.h"
#include "Action.h"
//#include "EBSDReader.h"

// Forward Declarations
class ReconVarICAction;

template<>
InputParameters validParams<ReconVarICAction>();

class ReconVarICAction: public Action
{
public:
  ReconVarICAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  unsigned int _op_num;
  std::string _var_name_base;
  //std::vector<VariableName> _eta;
};

#endif //RECONVARICACTION_H

