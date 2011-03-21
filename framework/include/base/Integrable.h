#ifndef INTEGRABLE_H_
#define INTEGRABLE_H_

class Integrable
{
public:
  virtual void computeResidual() = 0;
  virtual void computeJacobian(int /*i*/, int /*j*/) { }
};

#endif /* INTEGRABLE_H_ */
