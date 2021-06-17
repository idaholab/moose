# Include Extension

!include preamble.md start=We hold end=. include-end=True

!include preamble.md line=We hold

!include preamble.md re=(?P<content>Right\s.*?Government) indent=1 prepend=^ append=$ header=Header footer=Footer

> !include amendments.md
