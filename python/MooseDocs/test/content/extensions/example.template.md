# Template Extension

The {{project}} project is amazing!

## Field with Defaults

!template field key=field-with-default required=False
This is the default message, it is great.

## Field with Replacement

!template field key=field-with-item

## Field with Missing Replacement

!template field key=field-without-item
This item should be supplied, if it isn't then you get an error.
