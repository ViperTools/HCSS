import('smake/dependencyInstaller', true)
import('smake/dependencyIncluder', true)
local spinner = import('smake/enhancedSpinner')
local ninja = import('smake/ninja')

spinner.SetOptions({ symbols = 'clock' })

function smake.install()
    InstallDependency('hcss', function(installer)
        local folder = installer:GitClone('https://github.com/Syntad/syntad-css')
        folder:RunIn('smake')
        folder:MoveIncludeFolder()
        folder:MoveLibrary('libhcss.a')
        folder:Delete()
    end)
end

function smake.build()
    local ninjaGen = ninja.fromGlobalCompiler()

    IncludeDependency('hcss')
    flags('-O3')
    standard('c++20')
    include('include')
    inputr('src')

    spinner.Call(ninjaGen.generateBuildFile, 'Creating build file', '✅ Created Ninja file', ninjaGen, 'build')
    spinner.Call(run, 'Building', '✅ Built', 'ninja')

    smake.run()
end

function smake.run()
    run('build/a.out')
end