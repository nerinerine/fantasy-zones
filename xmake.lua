add_rules("mode.debug", "mode.release")
add_requires("libsdl3")
add_requires("fmt")
add_requires("lockfree")

target("fantasy_zones")
    set_languages("c++latest")
    set_kind("binary")
    set_toolchains("clang")

    add_packages("libsdl3")
    add_packages("fmt")
    add_packages("lockfree")

    add_headerfiles("src/**.hpp")
    add_files("src/**.cpp")
