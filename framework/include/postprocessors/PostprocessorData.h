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

//  std::map<std::string, Real> _values;

  PostprocessorValue & getPostprocessorValue(const std::string & name);

  void addData(const std::string & name, Real value, Real time);

  void print_table(std::ostream & out);
  void print_table(const std::string & file_name);
  void print_csv(const std::string & file_name);
  
private:  
  FormattedTable _output_table;
};
#endif //POSTPROCESSORDATA_H
