
#include "braceexpr.h"

#include <cstdlib>
#include <iostream>

namespace hit
{

std::string
BraceNode::str(int indent)
{
  std::string indstr;
  for (int i = 0; i < indent; i++)
    indstr += "    ";

  if (_val != "")
    return indstr + _val + "\n";

  std::string s = indstr + "${\n";
  for (auto & n : _list)
  {
    s += n.str(indent + 1);
  }
  s += indstr + "}\n";
  return s;
}

BraceNode &
BraceNode::append()
{
  _list.push_back(BraceNode());
  return _list.back();
}

std::string
EnvEvaler::eval(Field * /*n*/, const std::list<std::string> & args, BraceExpander & /*exp*/)
{
  std::string var = args.front();
  std::string val;
  auto data = std::getenv(var.c_str());
  if (data)
    val = data;
  return val;
}

std::string
RawEvaler::eval(Field * /*n*/, const std::list<std::string> & args, BraceExpander & /*exp*/)
{
  std::string s;
  for (auto & arg : args)
    s += arg;
  return s;
}

std::string
ReplaceEvaler::eval(Field * n, const std::list<std::string> & args, BraceExpander & exp)
{
  auto & var = args.front();
  Node * curr = n;
  while ((curr = curr->parent()))
  {
    auto src = curr->find(var);
    if (src && src != n && src->type() == NodeType::Field)
    {
      exp.used.push_back(pathJoin({curr->fullpath(), var}));
      // change kind only (not val)
      n->setVal(n->val(), dynamic_cast<Field *>(src)->kind());
      return curr->param<std::string>(var);
    }
  }

  exp.errors.push_back(errormsg(n, "no variable '", var, "' found for substitution expression"));
  return n->val();
}

size_t parseBraceNode(const std::string & input, size_t start, BraceNode & n);
size_t parseBraceBody(const std::string & input, size_t start, BraceNode & n);

void
BraceExpander::registerEvaler(const std::string & name, Evaler & ev)
{
  _evalers[name] = &ev;
}

void
BraceExpander::walk(const std::string & /*fullpath*/, const std::string & /*nodepath*/, Node * n)
{
  auto f = dynamic_cast<Field *>(n);
  if (!f)
    throw Error("BraceExpander cannot walk non-Field-type nodes");

  try
  {
    auto unquoted = f->val();
    auto quote = quoteChar(f->val());
    if (quote != "")
      unquoted = unquoted.substr(1, unquoted.size() - 2);

    std::string s;
    s = expand(f, unquoted);
    if (f->kind() == Field::Kind::String)
      s = quote + s + quote;

    if (errors.size() == 0)
      f->setVal(s);
  }
  catch (Error & err)
  {
    errors.push_back(errormsg(f, err.what()));
    return;
  }
}

std::string
BraceExpander::expand(Field * f, const std::string & input)
{
  std::string result = input;
  size_t start = 0;
  int count = 0;
  while ((start = result.find("${", start)) != std::string::npos)
  {
    BraceNode root;
    parseBraceNode(result, start, root);

    auto replace_text = expand(f, root);
    result.replace(root.offset(), root.len(), replace_text);
    start = root.offset() + replace_text.size();
    count++;
  }

  // switch kind back to string if there are multiple top-level brace expressions or there is text
  // outside of the single brace expression
  if (count > 1 || (count == 1 && (input.rfind("}") - input.find("${") + 1 < input.size())))
    // change kind only (not val)
    f->setVal(f->val(), Field::Kind::String);

  return result;
}

std::string
BraceExpander::expand(Field * n, BraceNode & expr)
{
  if (!expr.val().empty())
    return expr.val();

  auto args = expr.list();
  std::list<std::string> expanded_args;
  for (auto it = args.begin(); it != args.end(); ++it)
    expanded_args.push_back(expand(n, *it));

  // Just use replace expander if no args given
  if (expanded_args.size() == 1)
    return _replace.eval(n, expanded_args, *this);

  auto cmd = expanded_args.front();
  if (_evalers.count(cmd) == 0)
    throw hit::Error("invalid brace-expression command '" + cmd + "'");
  expanded_args.pop_front();
  return _evalers[cmd]->eval(n, expanded_args, *this);
}

size_t
skipSpace(const std::string & input, size_t start)
{
  const std::string space = "\n\t \r";
  while (start < input.size() && space.find(input[start]) != std::string::npos)
    start++;
  return start;
}

size_t
untilSpace(const std::string & input, size_t start)
{
  const std::string space = "\n\t \r}";
  while (start < input.size() && space.find(input[start]) == std::string::npos)
    start++;
  return start;
}

size_t
parseBraceNode(const std::string & input, size_t start, BraceNode & n)
{
  n.offset() = start;
  start += 2; // eat opening "${"
  start = parseBraceBody(input, start, n);
  start = skipSpace(input, start);
  if (input[start] != '}')
    throw Error("missing closing '}' in brace expression");
  start++; // eat closing "}"
  n.len() = start - n.offset();
  return start;
}

size_t
parseBraceBody(const std::string & input, size_t start, BraceNode & n)
{
  start = skipSpace(input, start);
  while (start < input.size() && input[start] != '}')
  {
    if (input.find("${", start) == start)
      start = parseBraceNode(input, start, n.append());
    else
    {
      auto end = untilSpace(input, start);
      auto & child = n.append();
      child.val() = input.substr(start, end - start);
      start = end;
    }
    start = skipSpace(input, start);
  }
  return start;
}

} // namespace hit
