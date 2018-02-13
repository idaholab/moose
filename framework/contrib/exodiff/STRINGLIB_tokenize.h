#ifndef TOKENIZE_H
#define TOKENIZE_H

#include <string>
#include <vector>

/**
 * Take the 'str' argument and split it using the list of characters
 * in separators as separators. Use tokens to return the result.
 */
namespace STRINGLIB {
  void tokenize(const std::string& str, const std::string& separators,
		std::vector<std::string>& tokens);
}

#endif
