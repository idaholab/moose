
from __future__ import print_function

import hit

n = hit.parse('foo', '[hello]world=42[]')
print(n.render())
print(n.type())
print(n.line())
print(n.raw())

n1 = hit.parse('foo', '[hello]world=42[]')
n2 = hit.parse('foo', 'foo=bar[hello]worlds=43[]')
print('before n1:')
print(n1)
print('before n2:')
print(n2)
hit.merge(n2, n1)
print('after n1:')
print(n1)
print('after n2:')
print(n2)
print(n2.kind())

kids = n2.children()
for kid in kids:
    print(kid.fullpath(), kid.param())

print(n1.param('hello/world'))
print(n1.param('hello'))
print(n1.param('hello/worlds'))
print(n1.param('foo'))
print(n1.find('hello/world').param())

try:
    hit.parse('bar', '[hello] world []')
except Exception as err:
    print(err)

