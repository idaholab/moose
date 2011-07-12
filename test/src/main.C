#include "MooseInit.h"
#include "Executioner.h"
#include "MooseTest.h"

// Parser
#include "Parser.h"
#include "MooseSyntax.h"
#include "MooseTestSyntax.h"

// libMesh includes
#include "perf_log.h"

// Create a performance log
PerfLog Moose::perf_log("Moose Test");

int main(int argc, char *argv[])
{
  MooseInit init(argc, argv);
  Parser p;
  
  MooseTest::registerObjects();
  // Associate Parser Syntax
  Moose::associateSyntax(p);
  MooseTest::associateSyntax(p);

  // Parse commandline and return inputfile filename if appropriate
  std::string input_filename = p.parseCommandLine();

  // Parse the input file
  p.parse(input_filename);
  p.execute();

  Executioner *e = p.getExecutioner();
  e->execute();
  delete e;

  return 0;
}
