rule("module.test")
    on_load(function (target)
        -- 没有开启 test 选项，就关闭 target
        if not has_config("test") then
            target:set("enabled", false)
            return
        end
        target:set("rundir", os.projectdir())
        target:set("group", "test")
        target:add("packages", "gtest")
    end)