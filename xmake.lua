add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
add_requires("jsoncpp")
add_requires("protobuf-cpp", "grpc", "openssl")
add_includedirs("include")
add_requires("boost",{configs = {all = true}})
add_requires("hiredis")
target("GateServer")
    set_languages("c++17")
    add_packages("jsoncpp")
    add_packages("boost")
    add_packages("protobuf-cpp", "grpc", "openssl")
    add_packages("hiredis")
    set_kind("binary")
    add_files("src/*.cpp")
    add_files("src/message.pb.cc", "src/message.grpc.pb.cc")
    -- 需要添加 Windows 套接字库 
    add_syslinks("ws2_32", "mswsock")
        -- 确保 Boost 路径被正确添加到编译命令
        -- 针对 MSVC 编译器添加 /utf-8 选项以消除 C4819 编码警告
    add_cxflags("/utf-8")
     -- 添加链接库搜索目录，假设hiredis.dll在C:/hiredis/lib目录下
    add_linkdirs("C:/Users/sheny/AppData/Local/.xmake/packages/h/hiredis/v1.3.0/eb2bb2120d0b41d793ca2984e0fb0eb2/bin") 
    -- xmake.lua

    -- 添加实际头文件路径
    -- add_includedirs("D:/cpp_lib_app/mysql-connect-cpp/include")
    -- add_linkdirs("D:/cpp_lib_app/mysql-connect-cpp/lib64/debug/vs14")
    -- add_links("mysqlcppconn", "mysqlcppconnx")

    -- after_build(function (target)
    --     local dll_src = "D:/cpp_lib_app/mysql-connect-cpp/lib64/debug/"
    --     local dll_dest = target:targetdir()
    --     os.cp(dll_src .. "mysqlcppconn-10-vs14.dll", dll_dest)
    --     os.cp(dll_src .. "mysqlcppconnx-2-vs14.dll", dll_dest)
    -- end)
    -- 针对 MSVC 编译器添加 /utf-8 选项以消除 C4819 编码警告
--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

