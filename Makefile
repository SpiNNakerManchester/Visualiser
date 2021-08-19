BUILDS = spynnaker_external_device_lib \
         c_based_visualiser_framework \
         spynnaker_external_device_lib/examples

DIRS = $(BUILDS)

all: $(DIRS)
	for d in $(DIRS); do "$(MAKE)" -C $$d || exit $$?; done

clean: $(DIRS)
	for d in $(DIRS); do "$(MAKE)" -C $$d clean || exit $$?; done
