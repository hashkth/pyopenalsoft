# Usage:
# make - build for all versions
# make VER=3.XX - build for 3.XX

ifneq ($(strip $(VER)),)
MAKEFLAGS += -j4
endif

VERSIONS := 3.11 3.12 3.13 3.14
VER ?=

PYTHON_SUFFIX = $(subst .,,$(VER))

ifneq ($(strip $(VER)),)
    PYTHON = py -$(VER)
    PY_PATH = $(subst \,/,$(shell $(PYTHON) -c "import sys; print(sys.prefix)"))
    PY_INCLUDE = $(PY_PATH)/include
    PYBIND11_INCLUDE = $(subst \,/,$(shell $(PYTHON) -c "import pybind11; print(pybind11.get_include())"))
endif

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
BACKUP_DIR = backups

TARGET = $(OUTPUT_DIR)/$(NAME)$(PYTHON_SUFFIX).pyd

.PHONY: all build multi-build build-version build-target clean

ifeq ($(strip $(VER)),)
all: multi-build
build: multi-build
else
all: build
build: build-version
endif

build-version:
	@echo.
	@echo Building library for Python $(VER)
	@echo ----------------------------------
	@$(MAKE) --no-print-directory VER=$(VER) build-target

build-target: $(TARGET)

multi-build: $(addprefix build-,$(subst .,,$(VERSIONS)))

define BUILD_VERSION
.PHONY: build-$(subst .,,$(1))

build-$(subst .,,$(1)):
	@$$(MAKE) -j4 --no-print-directory VER=$(1) build-version
endef

$(foreach v,$(VERSIONS),$(eval $(call BUILD_VERSION,$(v))))

$(TARGET): $(OBJ)
	@if not exist "$(OUTPUT_DIR)" mkdir "$(OUTPUT_DIR)"
	@echo Linking $@
	@$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	@echo BUILD SUCCESS: $(TARGET)
	@if not exist "$(BACKUP_DIR)" mkdir "$(BACKUP_DIR)"
	@powershell -NoProfile -Command "$$timestamp = Get-Date -Format 'yyyyMMdd_HHmmss'; $$destination = '$(BACKUP_DIR)/$(NAME)$(PYTHON_SUFFIX)_' + $$timestamp + '.pyd'; Copy-Item -LiteralPath '$(OUTPUT_DIR)/$(NAME)$(PYTHON_SUFFIX).pyd' -Destination $$destination"
	@echo Backup complete!

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if not exist "$(@D)" mkdir "$(@D)"
	@echo Compiling $<
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(OBJ:.o=.d)

clean:
	@if exist "obj" rmdir /s /q "obj"
	@echo Cleaning complete!