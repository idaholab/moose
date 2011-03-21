#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <string>
#include <vector>
#include "FormattedTable.h"
// libMesh
#include "libmesh_common.h"

namespace Moose {

class Problem;
class Outputter;

class Output {
public:
  Output(Problem & problem);
  virtual ~Output();

  void addExodus();

  void output();
  // FIXME: right now, it is here - might go somewhere else?
  void outputPps(const FormattedTable & table);

  void fileBase(const std::string & file_base) { _file_base = file_base; }
  std::string & fileBase() { return _file_base; }

  void interval(int interval) { _interval = interval; }
  int interval() { return _interval; }

  void meshChanged();
  void sequence(bool state);

protected:
  std::string _file_base;

  Problem & _problem;
  Real & _time;
  int _interval;

  std::vector<Outputter *> _outputters;
};

} // namespace

#endif /* OUTPUTTER_H_ */
