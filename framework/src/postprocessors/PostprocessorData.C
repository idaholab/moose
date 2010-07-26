#include "PostprocessorData.h"

//Moose includes
#include "MooseSystem.h"

PostprocessorData::PostprocessorData(MooseSystem & moose_system)
  :_moose_system(moose_system)
{}

bool
PostprocessorData::empty()
{
  return _values.empty();
}
