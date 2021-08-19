BUILDS = receiver_example \
         send_receive_example \
         sender_example \
         stepped_example

DIRS = $(BUILDS)

all: $(DIRS)
	for d in $(DIRS); do "$(MAKE)" -C $$d || exit $$?; done

clean: $(DIRS)
	for d in $(DIRS); do "$(MAKE)" -C $$d clean || exit $$?; done