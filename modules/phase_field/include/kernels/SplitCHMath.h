#ifndef SPLITCHMath_H
#define SPLITCHMath_H

#include "SplitCHCRes.h"


//Forward Declarations
class SplitCHMath;

template<>
InputParameters validParams<SplitCHMath>();

class SplitCHMath : public SplitCHCRes
{
public:

  SplitCHMath(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeDFDC(PFFunctionType type);

private:

};
#endif //SPLITCHMath_H
