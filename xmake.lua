add_rules("mode.debug", "mode.release")
add_requires("libsdl3")
add_requires("fmt")
add_requires("lockfree")
add_requires("yyjson")
add_requires("etl")

target("fantasy_zones")

    if is_mode("debug") then
        add_defines("DEBUG")
    end

    if is_mode("release") then
        add_ldflags("-Wl,--subsystem","-Wl,--entry:mainCRTStartup")
    end

    set_languages("c++latest")
    set_kind("binary")
    set_toolchains("clang")

    add_packages("libsdl3")
    add_packages("fmt")
    add_packages("lockfree")
    add_packages("yyjson")
    add_packages("etl")

    add_headerfiles("src/**.hpp")
    add_files("src/**.cpp")

    add_links("dwmapi")
