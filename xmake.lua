set_project("andirpg")
set_version("0.1.0")

set_config("cc", "clang")
set_config("ccache", false)

add_requires("cunit")
add_requires("inih", "ncurses")
add_cflags("-std=gnu2x", "-m64", "-xc")

set_defaultplat("linux")
set_defaultarchs("arm64-v8a")
set_allowedmodes("debug", "release")

if is_mode("debug") then
	set_optimize("none")
	set_symbols("debug")
	set_strip("none")
elseif is_mode("release") then
	set_optimize("fastest")
	set_symbols("none")
	set_strip("all")
	add_cflags("-fomit-frame-pointer")
end

add_packages("inih", "ncurses")
set_warnings("all", "error")
set_fpmodels("fast", "except")

target("engine")
set_kind("shared", { soname = true })
add_files("src/*.c")
add_includedirs("src")
target_end()

target("rpg")
set_kind("binary")
add_deps("engine")
add_files("rpg/*.c")
add_files("rpg/ui/*.c")
add_includedirs("src")
add_includedirs("rpg/ui")
target_end()

target("test")
set_kind("binary")
add_deps("engine")
add_defines("MAX_INVENTORY_SIZE=20")
add_packages("cunit")
add_files("test/*.c")
add_includedirs("src")
target_end()
