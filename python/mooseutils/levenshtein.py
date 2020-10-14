#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

def levenshteinDistance(s1, possible, number=None):
    """
    Return the sorted Levenstein distance of s1 against many strings.

    Input:
        s1: The string of interest
        possible: Possible string matches
        number: (Optional) The number of entries to return
    """
    results = []
    minimum = 1e9
    for i, s2 in enumerate(possible):
        d = levenshtein(s1, s2)
        results.append((s2, d))

    results = [r for r, _ in sorted(results, key=lambda x: x[1])]
    if number is not None:
        return results[:number]
    return results

def levenshtein(s1, s2):
    """
    Python implementation of Levenshtein algorithm
    https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#Python
    """

    if len(s1) < len(s2):
        return levenshtein(s2, s1)

    # len(s1) >= len(s2)
    if len(s2) == 0:
        return len(s1)

    previous_row = range(len(s2) + 1)
    for i, c1 in enumerate(s1):
        current_row = [i + 1]
        for j, c2 in enumerate(s2):
            insertions = previous_row[j + 1] + 1 # j+1 instead of j since previous_row and current_row are one character longer
            deletions = current_row[j] + 1       # than s2
            substitutions = previous_row[j] + (c1 != c2)
            current_row.append(min(insertions, deletions, substitutions))
        previous_row = current_row

    return previous_row[-1]
