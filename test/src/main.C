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

  std::string input_filename = "";
  if (Moose::command_line->search("-i"))
    input_filename = Moose::command_line->next(input_filename);
  else
    p.printUsage();

  // Associate Parser Syntax
  Moose::associateSyntax(p);
  MooseTest::associateSyntax(p);
  
  p.parse(input_filename);
  p.execute();

  Executioner *e = p.getExecutioner();
  e->execute();
  delete e;

  return 0;
}
