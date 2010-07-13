#ifndef FORMATTEDTABLE_H
#define FORMATTEDTABLE_H

#include "libmesh_common.h"

#include <string>
#include <map>
#include <set>
#include <ostream>


class FormattedTable
{
public:
  FormattedTable();

  /**
   * The destructor is used to close the file handle
   */
  ~FormattedTable();

  /**
   * Method for adding data to the output table.  The dependant varible is named "time"
   */
  void addData(const std::string & name, Real value, Real time);

  /**
   * Methods for dumping the table to the stream - either by filename or by stream handle.  If
   * a filename is supplied openening and closing of the file is properly handled
   */
  void print_table(std::ostream & out);
  void print_table(const std::string & file_name);

  /**
   * Method for dumping the table to a csv file - opening and closing the file handle is handled
   */
  void print_csv(const std::string & file_name);
  
private:
  
  void printRowDivider(std::ostream & out);
  
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

  /**
   * The single cell width used for all columns in the table
   */
  static const unsigned short _column_width;

  /**
   * The optional output file stream
   */
  std::ofstream _output_file;
  bool _stream_open;
};

  
#endif //FORMATTEDTABLE_H
