#include "FEProblem.h"

class SkipContractionTestProblem : public FEProblem
{
public:
  static InputParameters validParams();

  SkipContractionTestProblem(const InputParameters & params);

  bool allowMeshContraction() override final { return false; }
};
