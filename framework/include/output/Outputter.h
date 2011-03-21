#ifndef OUTPUTTER_H_
#define OUTPUTTER_H_

#include <string>

namespace Moose {

class Problem;

class Outputter {
public:
  Outputter(Problem & problem);
  virtual ~Outputter();

  /**
   * Outputs the data
   */
  virtual void output(const std::string & file_base) = 0;

protected:
  Problem & _problem;

  std::string _file_base;
};

} // namespace

#endif /* OUTPUTTER_H_ */
