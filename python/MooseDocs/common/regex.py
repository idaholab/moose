#pylint: disable=missing-docstring
import re
from exceptions import TokenizeException

def regex(pattern, content, flags=None, exc=TokenizeException):
    """
    Tool for searching for "content" within a regex and raising the desired exception if not found.
    """
    match = re.search(pattern, content, flags=flags)
    if match:
        if 'content' in match.groupdict():
            content = match.group('content')
        else:
            content = match.group()
    else:
        msg = "Failed to match regular expression: {}"
        raise exc(msg, pattern)

    return content
