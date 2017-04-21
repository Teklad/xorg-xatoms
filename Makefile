# Stuff for installation
EXECUTABLE = xatoms
PREFIX = /usr/local

# Source files
SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)

# Linker flags
LDFLAGS = -lxcb
CFLAGS = -O2 -s
# Default CFLAGS

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Done."
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: install
install: $(EXECUTABLE)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $< $(DESTDIR)$(PREFIX)/bin/$(EXECUTABLE)

.PHONY: uninstall
	rm -f $(DESTDIR)$(PREFIX)/bin/$(EXECUTABLE)
