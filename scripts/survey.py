#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Report statistics regarding the SQA documentation in MOOSE.
"""

try :
    import os
    import sys
    import hashlib
    import subprocess
except Exception :
    # if any import fails for any reason we just quit
    sys.exit(0)

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

home =os.getenv('HOME')
if home is None:
    # $HOME is not defined. Nope out.
    sys.exit(0)

base = home + '/.moose_survey'

# we allow users to opt out completely in case they have some weird hostname issues
if os.path.exists(base) :
    sys.exit(0)

# bail out early if curl is not available
#command -v curl > /dev/null 2>&1 || exit

bar = '=' * 70

# if user previously declined (empty config) we exit out right away
config = base + '.' + os.uname()[1]
if os.path.exists(config) and os.stat(config).st_size == 0 :
    sys.exit(0)

# machine hash
mhash = hashlib.md5(str(os.uname).encode('utf-8')).hexdigest()

# generate the compiler info
try :
    compiler_out = subprocess.check_output([sys.argv[1], "-v"], universal_newlines=True, stderr=subprocess.STDOUT)
except Exception :
    sys.exit(0)

# payload string to be reported back to INL
payload = "Anonymized machine id: %s\n%s" % (mhash, compiler_out)

# generate payload hash
phash = hashlib.md5(payload.encode('utf-8')).hexdigest()

# if the hash has been seen already we quit
if os.path.exists(config) :
    with open(config, 'r') as f:
        ohash = f.read()
    if ohash == phash :
        sys.exit(0)

# output information to the user
eprint(bar)
eprint("""The MOOSE team is considering raising the minimum compiler requirements
to enable support of C++17. You can help by allowing the MOOSE build
system  to report your compiler version, on this host ($(hostname))
back to us.

If you agree we would send the following data back to Idaho National
Laboratory:""")
eprint(bar)
eprint(payload)
eprint(bar)

# ask for the user reply
eprint('I agree to send the data to INL [y/N]: ')
consent = input()

if consent == 'y' or consent == 'Y' or consent == 'yes' :
    eprint("Thank you!")
    with open(config, 'w') as f:
        f.write(phash)
else :
    eprint("Ok :-(")
    open(config, 'a').close()
