## Moose Examples
all:: examples

dirlist := $(wildcard ex*/)

#
# Build the example programs
examples: $(dirlist)

$(dirlist):
	@$(MAKE) -C $@ $(MAKECMDGOALS)||exit 1


.PHONY: clean clobber distclean link run .depend test subdirs $(dirlist)

#
# Clean up the example programs
clean:
	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" $(MAKECMDGOALS)||exit 1; done

#
# Really clean up the example programs
clobber:
	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" $(MAKECMDGOALS)||exit 1; done

# Make the example program directories look
# like a clean distribution
distclean:
	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" $(MAKECMDGOALS)||exit 1; done

# Link the example programs
link: $(wildcard examples/ex*/*.C)
	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" all||exit 1; done

#
# Run the example programs to see if shared lib's get properly loaded
run: $(wildcard examples/ex*/*.C)
	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" $(MAKECMDGOALS)||exit 1; done

#
# Rebuild the dependencies for the examples
.depend: $(wildcard examples/ex*/*.C)
	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" $(MAKECMDGOALS)||exit 1; done

test:
	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" $(MAKECMDGOALS)||exit 1; done


