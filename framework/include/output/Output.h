#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <string>
#include <vector>

namespace Moose {

class SubProblem;
class Outputter;

class Output {
public:
  Output(SubProblem & problem);
  virtual ~Output();

  void addExodus();

  void output();

  void fileBase(const std::string & file_base) { _file_base = file_base; }

protected:
  std::string _file_base;

  SubProblem & _problem;

  std::vector<Outputter *> _outputters;
};

} // namespace

#endif /* OUTPUTTER_H_ */
