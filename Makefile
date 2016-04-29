BUILDS = spynnaker_external_device_lib c_based_visualiser_framework
DIRS = $(BUILDS)

all: $(DIRS)
	for d in $(DIRS); do (cd $$d; "$(MAKE)") || exit $$?; done

clean: $(DIRS)
	for d in $(DIRS); do (cd $$d; "$(MAKE)" clean) || exit $$?; done
