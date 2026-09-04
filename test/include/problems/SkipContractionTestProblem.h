#include "FEProblem.h"

class SkipContractionTestProblem : public FEProblem
{
public:
  static InputParameters validParams();

  SkipContractionTestProblem(const InputParameters & params);

  bool allowMeshContractionAfterMeshChanged() const override final { return false; }
};
