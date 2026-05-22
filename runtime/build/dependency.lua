local m = {}

local last_str = ''

function iop(str)
    io.write(('\b \b'):rep(#last_str)) -- erase old line
    io.write(str) -- write new line
    io.flush()
    last_str = str
end

newoption({ trigger = 'no-download-progress', description = 'Hide progress?' })

function m.github(project, tag)
    local dependencies = os.getenv('DEPENDENCIES')
    if dependencies == nil then
        dependencies = path.getabsolute(_WORKING_DIR) .. '/dependencies'
        os.mkdir(dependencies)
    end
    local dirname = project .. '_' .. tag
    dirname = string.gsub(dirname, '/', '_')
    local dependency_path = dependencies .. '/' .. dirname
    -- Check for .git rather than just the directory. These deps are tracked
    -- in the parent repo as gitlinks (mode 160000) with no .gitmodules
    -- registration, so `git checkout` materializes empty placeholder dirs.
    -- The original `os.isdir(dependency_path)` check skipped the clone in
    -- that case and the build died later with cryptic "no such file"
    -- errors on dep-internal paths (e.g.
    -- .../glennrp_libpng_libpng16/scripts/pnglibconf.h.prebuilt). Matches
    -- the same pattern used for premake-core in build_rive.sh.
    if not os.isdir(dependency_path .. '/.git') then
        os.rmdir(dependency_path)
        print('Fetching dependency ' .. project .. ' at tag ' .. tag .. '...')
        local gitcmd = 'git -c advice.detachedHead=false -C '
            .. dependencies
            .. ' clone --depth 1 --branch '
            .. tag
            .. ' https://github.com/'
            .. project
            .. '.git'
            .. ' '
            .. dirname
        if not os.execute(gitcmd) then
            error('\nError executing command:\n  ' .. cmd)
        end
    end
    assert(os.isdir(dependency_path))
    return dependency_path
end
return m
