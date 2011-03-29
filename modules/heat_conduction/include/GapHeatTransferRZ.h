#ifndef GAPHEATTRANSFERRZ_H
#define GAPHEATTRANSFERRZ_H

#include "GapHeatTransfer.h"


//Forward Declarations
class GapHeatTransferRZ;

template<>
InputParameters validParams<GapHeatTransferRZ>();

class GapHeatTransferRZ : public GapHeatTransfer
{
public:

  GapHeatTransferRZ(const std::string & name, InputParameters parameters);

  virtual ~GapHeatTransferRZ(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif //GAPHEATTRANSFERRZ_H
