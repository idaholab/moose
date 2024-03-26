#define BOOST_PARSER_DISABLE_HANA_TUPLE
#include <boost/parser/parser.hpp>
#include <iostream>
#include <map>

// anonymous namespace
namespace {

namespace bp = boost::parser;

// debug helper
template <typename T>
struct type_is;

enum class CapState
{
  FALSE,
  MAYBE_FALSE,
  MAYBE_TRUE,
  TRUE
};


const std::map<std::string, bool> caps = {{"petsc", true}, {"nope", false}};

// capability name
bp::rule<struct start_letter_tag, char> start_letter = "first letter of an identifier";
bp::rule<struct cont_letter_tag, char> cont_letter = "continuation of an identifier";
bp::rule<struct name_tag, std::string> name = "capability name";
auto const start_letter_def = bp::lower | bp::upper | '_';
auto const cont_letter_def = start_letter_def | bp::digit;
auto const name_def = start_letter >> *(cont_letter);
BOOST_PARSER_DEFINE_RULES(start_letter, cont_letter, name);

// check bool existence
const auto f_identifier = [](auto & ctx)
{
  if (caps.find(_attr(ctx)) != caps.end())
    _val(ctx) = CapState::TRUE;
  else
    _val(ctx) = CapState::MAYBE_FALSE;
};
const auto f_not_identifier = [](auto & ctx)
{
  if (caps.find(_attr(ctx)) != caps.end())
    _val(ctx) = CapState::FALSE;
  else
    _val(ctx) = CapState::MAYBE_TRUE;
};

const auto f_compare = [](auto & ctx)
{
  const auto & [left, op, right] = _attr(ctx);
  _val(ctx) = CapState::FALSE;
};

bp::symbols<int> const comparison = {
    {"<=", 0}, {">=", 1}, {"<", 2}, {">", 3}, {"!=", 4}, {"==", 5}, {"=", 5}};

// capability value
bp::rule<struct generic_tag, std::string> generic = "generic capability value";
bp::rule<struct version_tag, std::vector<unsigned int>> version = "version number";
bp::rule<struct value_tag, std::variant<std::string, std::vector<unsigned int>>> value =
    "capability value";
auto const generic_def = +(bp::lower | bp::upper | bp::digit | '_' | '-');
auto const version_def = bp::uint_ >> *('.' >> bp::uint_);
auto const value_def = generic | version;
// auto const value_def = +(bp::lower | bp::upper | '_' | '-') | (bp::digit >> *('.' >>
// +bp::digit));
BOOST_PARSER_DEFINE_RULES(generic, version, value);

// bool_statement
bp::rule<struct bool_statement_tag, CapState> bool_statement = "bool statement";
auto const bool_statement_def =
    ('!' >> name)[f_not_identifier] | name[f_identifier] | (name >> comparison >> value)[f_compare];
BOOST_PARSER_DEFINE_RULES(bool_statement);

const auto f_and = [](auto & ctx)
{
  const auto & [left, right] = _attr(ctx);
  const auto states = {
      CapState::FALSE, CapState::MAYBE_FALSE, CapState::MAYBE_TRUE, CapState::TRUE};

  for (const auto state : states)
    if (left == state || right == state)
    {
      _val(ctx) = state;
      return;
    }
};

const auto f_or = [](auto & ctx)
{
  const auto & [left, right] = _attr(ctx);
  const auto states = {
      CapState::TRUE, CapState::MAYBE_TRUE, CapState::MAYBE_FALSE, CapState::FALSE};

  for (const auto state : states)
    if (left == state || right == state)
    {
      _val(ctx) = state;
      return;
    }
};

const auto f_negate = [](auto & ctx)
{
  // negate current capability state
  switch (_attr(ctx))
  {
    case CapState::FALSE:
      _val(ctx) = CapState::TRUE;
      break;
    case CapState::TRUE:
      _val(ctx) = CapState::FALSE;
      break;
    case CapState::MAYBE_FALSE:
      _val(ctx) = CapState::MAYBE_TRUE;
      break;
    case CapState::MAYBE_TRUE:
      _val(ctx) = CapState::MAYBE_FALSE;
      break;
  }
};

const auto f_pass = [](auto & ctx)
{
  // pass through value
  _val(ctx) = _attr(ctx);
};

// expression
bp::rule<struct p_conjunction_tag, CapState> p_conjunction = "conjunction expression";
bp::rule<struct p_negate_tag, CapState> p_negate = "boolean negation";
bp::rule<struct p_pass_tag, CapState> p_pass = "boolean statement";
bp::rule<struct expr_tag, CapState> expr = "boolean expression";
auto const p_conjunction_def = (expr >> "&" >> expr)[f_and] | (expr >> "|" >> expr)[f_or];
auto const p_negate_def = "!(" >> expr >> ")";
auto const p_pass_def = ("(" >> expr >> ")") | bool_statement;
auto const expr_def = p_negate[f_negate] | p_pass[f_pass] | p_conjunction; //[f_conjunction];
BOOST_PARSER_DEFINE_RULES(p_conjunction, p_negate, p_pass, expr);
}

/*
<name> ::= [a-z] ([a-z] | [0-9])*
<comp> ::= (">" | "<") ("=")? | "!=" | "="
<conj> ::= "&" | "|"
<value> ::= ([a-z] | [0-9] | "_" | "-")+ | [0-9] ("." [0-9]+)*
<bool> ::= <name> | <name> <comp> <value>
<expr> ::= "!(" <expr> ")" | "(" <expr> ")" | <bool> | <expr> <conj> <expr>

https://bnfplayground.pauliankline.com/?bnf=%3Cname%3E%20%3A%3A%3D%20%5Ba-z%5D%20(%5Ba-z%5D%20%7C%20%5B0-9%5D)*%0A%3Ccomp%3E%20%3A%3A%3D%20(%22%3E%22%20%7C%20%22%3C%22)%20(%22%3D%22)%3F%20%7C%20%22!%3D%22%20%7C%20%22%3D%22%0A%3Cconj%3E%20%3A%3A%3D%20%22%26%22%20%7C%20%22%7C%22%0A%3Cvalue%3E%20%3A%3A%3D%20(%5Ba-z%5D%20%7C%20%5B0-9%5D%20%7C%20%22_%22%20%7C%20%22-%22)%2B%20%7C%20%5B0-9%5D%20(%22.%22%20%5B0-9%5D%2B)*%20%0A%3Cbool%3E%20%3A%3A%3D%20%3Cname%3E%20%7C%20%3Cname%3E%20%3Ccomp%3E%20%3Cvalue%3E%0A%3Cexpr%3E%20%3A%3A%3D%20%22!(%22%20%3Cexpr%3E%20%22)%22%20%7C%20%22(%22%20%3Cexpr%3E%20%22)%22%20%7C%20%3Cbool%3E%20%7C%20%3Cexpr%3E%20%3Cconj%3E%20%3Cexpr%3E&name=Real%20Numbers*/
int
main()
{
  std::string input = "23.4.12";

  auto const result = bp::parse(input, expr, bp::ws);
  // type_is<decltype(*result)>();

  // if (result)
  // {
  //   std::cout << "Great! It looks like you entered:\n";
  //   // std::cout << *result << "\n";
  //   for (auto & x : *result)
  //   {
  //     std::cout << x << "\n";
  //   }
  // }
  // else
  // {
  //   std::cout << "Good job!  Please proceed to the recovery annex for cake.\n";
  // }

  return 0;
}
