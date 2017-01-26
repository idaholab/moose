#!/bin/bash

mac_names="mooseWarning mooseDeprecated mooseError EFAError EFAWarning mooseInfo"

for i in $(seq 12); do
    for mac in $mac_names; do
        ag -l "$mac"' *\(' . | grep -v 'MooseError' | xargs perl -0777 -i'' -pe 's/('"$mac"' *\(((?!\) ?;).)+? *?) ?<< ?(.+?(?=\) ?;))/\1, \3/igs'
        #grep --recursive -l "$mac"' *(' . | grep -v 'MooseError' | xargs perl -0777 -i'' -pe 's/('"$mac"' *\(((?!\) ?;).)+? *?) ?<< ?(.+?(?=\) ?;))/\1, \3/igs'
    done
done

mac_names="mooseWarning mooseDeprecated mooseError mooseInfo"

for mac in $mac_names; do
    ag -l "$mac" . | grep -v 'MooseError' | xargs sed -i'' -e 's/'"$mac"'\([^a-zA-Z0-9]\)/'"$mac"'2\1/g'
    #grep --recursive -l "$mac" . | grep -v 'MooseError' | xargs sed -i'' -e 's/'"$mac"'\([^a-zA-Z0-9]\)/'"$mac"'2\1/g'
done

#ag -l "$mac" . | grep -v 'MooseError' | xargs perl -0777 -i'' -pe 's/( *\\\?)\n( *),/,\n \2/igs'

