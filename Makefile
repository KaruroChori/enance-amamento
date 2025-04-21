# Default prefix; override with command line, e.g. "make install PREFIX=/usr"
PREFIX ?= /usr/local

# Build directory for Meson
BUILD_DIR ?= build

# Meson command, use muon if possible
MESON ?= $(shell command -v muon >/dev/null 2>&1 && echo muon || echo meson)

BUILDTYPE ?= release

TOOLCHAIN ?= platforms/clang.ini

OFFLOAD ?= amd64-linux-unknown

NO_OMP ?= false

USE_USM ?= false

.PHONY: all configure build install dist test clean docs

# Run all steps by default.
all: configure build install docs

# Configure step.
configure:
	@echo "Configuring project with $(MESON) (build type: $(BUILDTYPE))..."
	$(MESON) setup $(BUILD_DIR) -Dno-omp=$(NO_OMP) -Duse-usm=$(USE_USM) -Doffload-targets=$(OFFLOAD) --reconfigure --prefix=$(PREFIX) --buildtype=$(BUILDTYPE) --native-file $(TOOLCHAIN) 

# Build step depends on configuration.
build: configure
	@echo "Building project with $(MESON)..."
	$(MESON) compile -C $(BUILD_DIR)

# Install step depends on building.
install: build
	@echo "Installing project with $(MESON)..."
	$(MESON) install -C $(BUILD_DIR)

test: build
	$(MESON) test -C $(BUILD_DIR)

# Dist target: Create a distribution tarball.
dist: install
	@echo "Creating tarball of the project via $(MESON)..."
	$(MESON) dist -C $(BUILD_DIR)

# Run the main app setting up the environment. Very much needed as the base stack size is extremely small and segfaults.
run: configure
	@echo "Running with default env vars"
	$(MESON) compile enamento-demo -C $(BUILD_DIR)
	LIBOMPTARGET_STACK_SIZE=2048 ./build/src/app/enamento-demo 

docs:
	doxygen ./dist/configs/Doxyfile
	mkdocs build -f ./dist/configs/mkdocs.yml

# Clean up the build directory.
clean:
	@echo "Cleaning up build directory..."
	rm -rf $(BUILD_DIR)