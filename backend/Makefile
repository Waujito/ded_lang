BUILD_DIR := build

STATIC_LIB := $(BUILD_DIR)/tasks_lib.a
STATIC_LIB_TARGET := ../deps/ded_tasks

SHARED_LIB := $(BUILD_DIR)/vlvm_shared_lib.a
SHARED_LIB_TARGET := ../shared/

SANITIZER_FLAGS := -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

CFLAGS := -D _DEBUG -ggdb3 -O0 -Wall -Wextra -Waggressive-loop-optimizations -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts  -Wconversion -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wopenmp-simd -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-missing-field-initializers -Wno-narrowing -Wno-varargs -Wstack-protector -fcheck-new -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla -Iinclude -D _GNU_SOURCE $(SANITIZER_FLAGS) -I$(STATIC_LIB_TARGET)/include -I$(SHARED_LIB_TARGET)/include

CXXFLAGS := $(CFLAGS) -Weffc++ -Wc++14-compat -Wconditionally-supported -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel -Wsuggest-override -Wno-literal-suffix -Wno-old-style-cast -std=c++17 -fsized-deallocation

CXX := g++
CC := gcc
FLAGS = $(CXXFLAGS)

LDFLAGS := -lm

TESTSRC := test/test_lexer.cpp test/test_parser.cpp
TESTOBJ := $(TESTSRC:%.cpp=$(BUILD_DIR)/%.cpp.o)
TEST_LIB_APP := $(BUILD_DIR)/test_backend

LIBSRC := src/backend.c
LIBOBJ := $(LIBSRC:%.c=$(BUILD_DIR)/%.c.o)
BACKEND_LIB := $(BUILD_DIR)/backend_lib.a

BACKEND_SRC := src/backend_main.c 
BACKEND_OBJ := $(BACKEND_SRC:%.c=$(BUILD_DIR)/%.c.o)
BACKEND_APP := $(BUILD_DIR)/backend

INCPDSRC := $(LIBSRC) $(BACKEND_SRC)
INCPDSRC_CPP := $(TESTSRC)
incpd := $(INCPDSRC:%.c=$(BUILD_DIR)/%.c.d) $(INCPDSRC_CPP:%.cpp=$(BUILD_DIR)/%.cpp.d)

OBJFILES := $(LIBOBJ) $(TESTOBJ) $(BACKEND_OBJ)
OBJDIRS := $(sort $(dir $(OBJFILES)))

.PHONY: build clean run test document build_test objdirs

build: $(BACKEND_APP) $(STATIC_LIB)

backend: $(BACKEND_APP)
	./$(BACKEND_APP)

$(OBJDIRS):
	mkdir -p $(OBJDIRS)

$(filter %.cpp.o,$(OBJFILES)): $(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(OBJDIRS)
	# $< takes only the FIRST dependency
	$(CXX) $(FLAGS) $(OBJCFLAGS) -MP -MMD -c $< -o $@

$(filter %.c.o,$(OBJFILES)): $(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(OBJDIRS)
	# $< takes only the FIRST dependency
	$(CC) $(CFLAGS) $(OBJCFLAGS) -MP -MMD -c $< -o $@

$(BACKEND_LIB): $(LIBOBJ)
	ar rvs $@ $^

.PHONY: $(STATIC_LIB)
$(STATIC_LIB): $(OBJDIRS)
	$(MAKE) -C $(STATIC_LIB_TARGET)
	cp $(STATIC_LIB_TARGET)/build/tasks_lib.a $(STATIC_LIB)

.PHONY: $(SHARED_LIB)
$(SHARED_LIB): $(OBJDIRS) $(STATIC_LIB_TARGET)
	$(MAKE) -C $(SHARED_LIB_TARGET)
	cp $(SHARED_LIB_TARGET)/build/vlvm_shared_lib.a $(SHARED_LIB)

$(TEST_LIB_APP): $(STATIC_LIB) $(SHARED_LIB) $(BACKEND_LIB) $(TESTOBJ)
	$(CXX) $(FLAGS) $(LDFLAGS) $(TESTOBJ) $(BACKEND_LIB) $(SHARED_LIB) $(STATIC_LIB) -lgtest_main -lgtest -o $(TEST_LIB_APP)

build_test: $(TEST_LIB_APP)

test: build_test
	./$(TEST_LIB_APP)

$(BACKEND_APP): $(BACKEND_OBJ) $(BACKEND_LIB) $(STATIC_LIB) $(SHARED_LIB)
	$(CXX) $(FLAGS) $(LDFLAGS) $(BACKEND_OBJ) $(BACKEND_LIB) $(SHARED_LIB) $(STATIC_LIB) -o $@

document: objdirs
	doxygen doxygen.conf

clean:
	rm -rf build

distclean: clean
	$(MAKE) -C $(STATIC_LIB_TARGET) clean

-include $(incpd)
