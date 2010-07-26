#ifndef POSTPROCESSORDATA_H
#define POSTPROCESSORDATA_H

//MOOSE includes
#include "Moose.h"
#include "MooseArray.h"
#include "Postprocessor.h"
#include "FormattedTable.h"

//libMesh includes
#include "transient_system.h"

//Forward Declarations
class MooseSystem;

class PostprocessorData
{
public:
  bool empty();
  
  PostprocessorData(MooseSystem & moose_system);

  MooseSystem & _moose_system;

  std::map<std::string, Real> _values;

  FormattedTable _output_table;
};
#endif //POSTPROCESSORDATA_H
