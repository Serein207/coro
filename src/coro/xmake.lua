add_requires("liburing")

target("coro")
    set_kind("static")
    set_languages("c++23")
    add_files("**.cpp")
    add_packages("liburing")
    add_includedirs(path.join(os.projectdir(), "include"), { public = true })
