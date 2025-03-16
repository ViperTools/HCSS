local spinner = import('smake/enhancedSpinner')
local ninja = import('smake/ninja')

spinner.SetOptions({ symbols = 'clock' })

function smake.build()
    local ninjaGen = ninja.fromGlobalCompiler()

    flags('-O3')
    standard('c++20')
    inputr('CSS')

    spinner.Call(ninjaGen.generateBuildFile, 'Creating build file', '✅ Created Ninja file', ninjaGen, 'build')
    spinner.Call(run, 'Building', '✅ Built', 'ninja')

    smake.run()
end

function smake.run()
    run('build/a.out')
end