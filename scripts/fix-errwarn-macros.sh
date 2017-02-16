#!/bin/bash

for i in $(seq 12); do
    ag -l "(moose(Error|Warning|Info|Deprecated)|EFAError|EFAWarning)"'2* *\(' . | grep -v 'MooseError' | xargs perl -0777 -i'' -pe 's/moose((Warning|Error|Deprecated|Info)2*)( *\(((?!\) ?;).)+? *?) ?<< ?(.+?(?=\) ?;))/moose${2}2$3, $5/igs'
done

# fix leading commas to be trailing
#ag -l "moose(Error|Warning|Info|Deprecated)" . | grep -v 'MooseError' | xargs perl -0777 -i'' -pe 's/( *\\\?)\n( *),/,\n \2/igs'

# civet precheck regexp
#'s/moose(Warning|Error|Deprecated|Info)( *\(((?!\) ?;).)+? *?) ?<< ?(.+?(?=\) ?;))/igs'
