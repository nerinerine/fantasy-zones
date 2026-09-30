add_rules("mode.debug", "mode.release")
add_requires("libsdl3")
add_requires("fmt")
add_requires("yyjson")
add_requires("etl")

target("fantasy_zones")

    set_languages("c++latest")
    set_kind("binary")
    set_toolchains("clang")

    if is_mode("debug") then
        add_defines("DEBUG")
        add_rules("win.sdk.console")
    else
        -- 去除程序运行时的命令行窗口
        add_rules("win.sdk.application")
        add_ldflags("-Wl,-entry:mainCRTStartup")
        -- 启用链接时优化
        -- add_cxflags("-flto")
        -- add_ldflags("-flto")
        -- 加入一点优化参数
        add_cxflags("-mtune=generic")
        add_cxflags("-ffast-math")
        add_cxflags("-fomit-frame-pointer","-fmerge-all-constants")
        add_cxflags("-ffunction-sections","-fdata-sections")
        add_ldflags("-Wl,--gc-sections", "-s")
        -- 去除c++一些幽默的特性
        add_cxxflags("-fno-rtti")
        add_cxflags("-fno-exceptions")
        -- 倾向于体积优化
        set_optimize("smallest")
    end

    add_packages("libsdl3")
    add_packages("fmt")
    add_packages("yyjson")
    add_packages("etl")

    add_headerfiles("src/**.hpp")
    add_files("src/**.cpp")

    add_links("dwmapi")
