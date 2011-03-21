#ifndef OUTPUTTER_H_
#define OUTPUTTER_H_

#include <string>

namespace Moose {
class Problem;
}

class Outputter {
public:
  Outputter(Moose::Problem & problem);
  virtual ~Outputter();

  /**
   * Outputs the data
   */
  virtual void output(const std::string & file_base) = 0;

protected:
  Moose::Problem & _problem;

  std::string _file_base;
};

#endif /* OUTPUTTER_H_ */
