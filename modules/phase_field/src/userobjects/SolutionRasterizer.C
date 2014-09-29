#include "SolutionRasterizer.h"

template<>
InputParameters validParams<SolutionRasterizer>()
{
  InputParameters params = validParams<SolutionUserObject>();
  return params;
}

SolutionRasterizer::SolutionRasterizer(const std::string & name, InputParameters parameters) :
    SolutionUserObject(name, parameters)
{
}

void
SolutionRasterizer::initialSetup()
{
  // only execute once
  if (_initialized) return;

  // initialize parent class
  SolutionUserObject::initialSetup();

  std::cout << "SolutionRasterizer::initialSetup() doing stuff!\n";

  for (Real x = 0.0; x < 1.0; x += 0.5)
    for (Real y = 0.0; y < 1.0; y += 0.5)
      for (Real z = 0.0; z < 1.0; z += 0.5)
        std::cout << x << ' ' << y << ' ' << z << ' ' << pointValue(0.0, Point(x,y,z), "c") << '\n';
}
