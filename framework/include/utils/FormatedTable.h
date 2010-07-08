#ifndef FORMATEDTABLE_H
#define FORMATEDTABLE_H

#include "libmesh_common.h"

#include <string>
#include <map>
#include <set>
#include <ostream>


class FormatedTable
{
public:
  FormatedTable();

  void addData(const std::string & name, Real value, Real time);

  void print(std::ostream & out);
  
private:
  /**
   * Data structure for the console table
   * The first map creates an association from the independent variable (normally time)
   * to a map of dependant variables and their associated values if they exist
   */
  std::map<Real, std::map<std::string, Real> > _data;

  /**
   * The set of column names updated when data is inserted through the setter methods
   */
  std::set<std::string> _column_names;
};

  
#endif //FORMATEDTABLE_H
