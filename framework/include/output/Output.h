#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <string>
#include <vector>

namespace Moose {

class Problem;
class Outputter;

class Output {
public:
  Output(Problem & problem);
  virtual ~Output();

  void addExodus();

  void output();

  void fileBase(const std::string & file_base) { _file_base = file_base; }

protected:
  std::string _file_base;

  Problem & _problem;

  std::vector<Outputter *> _outputters;
};

} // namespace

#endif /* OUTPUTTER_H_ */
