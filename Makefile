# Les avertissements ne sont pas inclus car normalement ils ont été vérifiés
# pendant le développement.

COMMON_CFLAGS = \
 -pthread \
 $(NULL)

PROD_CFLAGS = \
 -O3 \
 $(COMMON_CFLAGS) \
 $(JSON_CFLAGS) \
 $(NULL)

DEV_CFLAGS = \
 -Wall \
 -Wextra \
 -std=c99 \
 -pedantic \
 -Werror \
 -fsanitize=address,bounds,null,integer,undefined \
 -g \
 $(COMMON_CFLAGS) \
 $(JSON_CFLAGS) \
 $(NULL)

PROD_LDFLAGS = \
  $(JSON_LDLAGS) \
  $(NULL)

DEV_LDFLAGS = \
  $(JSON_LDLAGS) \
  $(NULL)

SRC = \
 src/convert_name.c \
 src/errors.c \
 src/main.c \
 src/parsing/dns_reader.c \
 src/parsing/hashmap.c \
 src/parsing/node.c \
 src/parsing/parse_json.c \
 src/serialization/dns_message.c \
 src/server/ip_tools.c \
 src/server/server.c \
 $(NULL)

OBJS = $(SRC:src/%.c=build/$(MODE)/%.o)

DEPS = $(OBJS:.o=.d)

DIRS = $(dir $(OBJS))

MODE ?= PROD

BINARY = build/$(MODE)/dnsd

reverse = $(shell printf "%s\n" $(strip $1) | tac)

.PHONY: all
all: $(BINARY)

$(BINARY): $(OBJS) | build/$(MODE)
	$(CC) $($(MODE)_CFLAGS) $($(MODE)_CPPFLAGS) $($(MODE)_LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

build/:
	mkdir "$@"

.PHONY: test
test: $(BINARY)
	python3 tests/run_tests.py


.PHONY: clean
clean:
	$(RM) $(OBJS) $(BINARY) MakefileConfig iwyu format tidy
ifeq ($(MODE),DEV)
	$(RM) $(DEPS)
endif
	rmdir $(call reverse, $(DIRS)) build/ 2>/dev/null || true

MakefileConfig: configure
	./configure > "$@"

# Ignore the file as it serve of marker so find can skip old files.
# This script avoid us to generate a marker for each file and derivate a special
# marker
.PHONY: format
format:
	@echo '===== Begin fomatting ====='
	@if [ -e "$@" ]; \
	then \
	  find src -newer "$@" \( -name '*.c' -o -name '*.h' \) \
    -exec clang-format -i -style=file \{\} \+ -print; \
	else \
	  find src \( -name '*.c' -o -name '*.h' \) \
    -exec clang-format -i -style=file \{\} \+ -print; \
	fi
	@echo '=====  End fomatting  ====='
	@touch "$@"

.PHONY: tidy
tidy: $(SRC)
	clang-tidy --format-style=file --fix -header-filter=.* --quiet $?
	touch tidy

iwyu: $(SRC)
	for file in src/*/*.h $?; do include-what-you-use -Xiwyu \
  --no_fwd_decls $($(MODE)_CPPFLAGS) $($(MODE)_CFLAGS) $$file; done > iwyu

.PHONY: install
install: all
	sudo install -m 700 build/$(MODE)/dnsd /usr/bin/dnsd
	sudo install -m 700 dnsd.service /usr/lib/systemd/system/dnsd.service
include MakefileConfig
-include $(DEPS)

.SECONDARY: $(DIRS)

.SECONDEXPANSION:
# here $$ $(VARIABLE) $(function) get expanded first
# then %
# then $$ $(VARIABLE) $(function) get expanded again thanks to SECONDEXPANSION
# this only apply in prerequisite lists defined after SECONDEXPANSION
# so please add rule before unless needed

build/DEV/%.o: DEV_CPPFLAGS += -MD -MT "$@" -MF "$(@:.o=.d)"
build/$(MODE)/%.o: src/%.c | $$(dir build/$(MODE)/%)
	$(CC) $($(MODE)_CFLAGS) $($(MODE)_CPPFLAGS) $(TARGET_ARCH) -c $(OUTPUT_OPTION) $<

build/%.d:
# Spécial hack to support GNU Make 3.81
	@:

build/%/: | $$(dir build/%)
	mkdir "$@"
