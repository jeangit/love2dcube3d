# $$DATE$$ : ven. 13 juillet 2018 (10:00:05)


love_version := $(shell love --version | sed 's/^[^0-9]*\([0-9]\+\).*/\1/g')
love_needed := 11

ifneq ($(love_needed), $(love_version))
$(error love version $(love_version) incorrect. Must be $(love_needed))
endif


all: libglm.so
	love .

libglm.so: lua_glm.cpp
	@g++ $< -fvisibility=hidden -fPIC -shared -Wall -W -lluajit-5.1 -DLUAJIT -o $@
