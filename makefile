
# Usage:
# make - build for all versions
# make VER=3.XX - build for 3.XX

VERSIONS := 3.11 3.12 3.13
VER ?=

PYTHON_SUFFIX = $(subst .,,$(VER))
PYTHON = py -$(VER)

PY_PATH = $(shell $(PYTHON) -c "import sys; print(sys.prefix)")
PY_INCLUDE = $(PY_PATH)/include
PYBIND11_INCLUDE = $(shell $(PYTHON) -c "import pybind11; print(pybind11.get_include())")

CXX := g++
CXXFLAGS := -O3 -Wall -std=c++17
CPPFLAGS = -Iinclude \
           -I"$(PY_INCLUDE)" \
           -I"$(PYBIND11_INCLUDE)"

LDFLAGS := -shared -static-libgcc -static-libstdc++
LDLIBS = -L"$(PY_PATH)/libs" -lpython$(PYTHON_SUFFIX)

DEPFLAGS := -MMD -MP

NAME := pyopenalsoft
SRC_DIR := src
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_DIR = obj/$(PYTHON_SUFFIX)
OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))
OUTPUT_DIR = release

TARGET = $(OUTPUT_DIR)/$(NAME)$(PYTHON_SUFFIX).pyd

.PHONY: all multi-build build-one build clean

ifeq ($(strip $(VER)),)
all: multi-build
else
all: build-one
endif

define BUILD_VERSION
.PHONY: build-$(subst .,,$(1))

build-$(subst .,,$(1)):
	@echo.
	@echo Building library for Python $(1)
	@echo ----------------------------------
	@$$(MAKE) --no-print-directory VER=$(1) build
endef

$(foreach v,$(VERSIONS),$(eval $(call BUILD_VERSION,$(v))))

multi-build: $(addprefix build-,$(subst .,,$(VERSIONS)))

build: $(TARGET)

$(TARGET): $(OBJ)
	@if not exist "$(OUTPUT_DIR)" mkdir "$(OUTPUT_DIR)"
	@echo Linking $@
	@$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	@echo BUILD SUCCESS: $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
	@echo Compiling $<
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(OBJ:.o=.d)

clean:
	@if exist "obj" rmdir /s /q "obj"
	@echo Cleaning complete!