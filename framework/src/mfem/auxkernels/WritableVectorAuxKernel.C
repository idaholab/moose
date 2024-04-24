#include "WritableVectorAuxKernel.h"

registerMooseObject("MooseApp", WritableVectorAuxKernel);

MooseVariable &
WritableVectorAuxKernel::writableVariable(const std::string & var_name, unsigned int comp)
{
  auto * var = dynamic_cast<MooseVariable *>(getVar(var_name, comp));

  // Make sure only one object can access a variable.
  checkWritableVar(var);

  return *var;
}
