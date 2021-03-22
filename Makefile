SUBDIR=directfb file linuxfb

BUILDSUBDIR = $(SUBDIR:%=build-%)
CLEANSUBDIR = $(SUBDIR:%=clean-%)

.PHONY: all $(SUBDIR) $(BUILDSUBDIR) $(CLEANSUBDIR)

all: $(BUILDSUBDIR)
$(BUILDSUBDIR):
	$(MAKE) -C $(@:build-%=%)

clean: $(CLEANSUBDIR)
$(CLEANSUBDIR):
	$(MAKE) -C $(@:clean-%=%) clean
