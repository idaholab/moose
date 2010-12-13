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

#ifndef POSTPROCESSORDATA_H
#define POSTPROCESSORDATA_H

//MOOSE includes
#include "Moose.h" // for PostprocessorValue
#include "FormattedTable.h"

//Forward Declarations
class MooseSystem;

class PostprocessorData
{
public:
  bool empty();
  
  PostprocessorData(MooseSystem & moose_system);

  void init(const std::string & name);

  PostprocessorValue & getPostprocessorValue(const std::string & name);

  void storeValue(const std::string & name, Real value);

  void addData(const std::string & name, Real value, Real time);

  void printTable(std::ostream & out);
  void printTable(const std::string & file_name);
  void printCSV(const std::string & file_name);
  void printEnsight(const std::string & file_name);
  void writeExodus(ExodusII_IO * ex_out, Real time);
  void makeGnuplot(const std::string & file_name, const std::string & format);
  
private:
  MooseSystem & _moose_system;
  FormattedTable _output_table;
  std::map<std::string, Real> _values;
};
#endif //POSTPROCESSORDATA_H
