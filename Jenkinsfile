properties([
        disableConcurrentBuilds(),
        buildDiscarder([$class: 'EnhancedOldBuildDiscarder', artifactDaysToKeepStr: '7', artifactNumToKeepStr: '10', daysToKeepStr: '365', discardOnlyOnSuccess: false, numToKeepStr: '']),
        pipelineTriggers([upstream('docker'),cron('@weekly'), pollSCM('@hourly')])
])

node("cmake && iwyu && cppcheck && clangtidy") {
    checkout scm
    def cmake = sh returnStdout: true, script: 'which cmake3 cmake | head -n1'
    def ctest = sh returnStdout: true, script: 'which ctest3 ctest | head -n1'
    sh 'chmod a+x scripts/*'
    withEnv(["CMAKE=${cmake}", "CTEST=${ctest}", "IGNORE_EXIT_CODE=1"]) {
        dir("build") {
            cache(caches: [[$class: 'ArbitraryFileCache', excludes: '', includes: '**/*.zip,**/*.tar.gz', path: 'external']], maxCacheSize: 100) {
                sh 'nice ${CMAKE} -G Ninja ../ -DBUILD_TESTING=ON -DVALGRIND_XML=ON -DCPACK_BINARY_RPM=ON -DCMAKE_BUILD_TYPE=Debug -DCLANG_TIDY=ON'
                sh 'nice ${CMAKE} --build . -- -j2 -l15 all'
                sh 'nice ${CMAKE} --build . -- -j2 -l15 checks'
                catchError {
                    sh 'nice ${CTEST} --verbose .'
                }
                sh '${CMAKE} --build . -- package'
                junit '**/*-unit.xml'
                archiveArtifacts '**/*valgrind.xml'
                step([$class        : 'XUnitBuilder',
                      testTimeMargin: '3000',
                      thresholdMode : 1,
                      thresholds    : [
                              [$class: 'FailedThreshold', failureNewThreshold: '', failureThreshold: '', unstableNewThreshold: '', unstableThreshold: '0'],
                              [$class: 'SkippedThreshold', failureNewThreshold: '', failureThreshold: '', unstableNewThreshold: '', unstableThreshold: '0']
                      ],
                      tools         : [
                              [$class               : 'ValgrindJunitHudsonTestType',
                               deleteOutputFiles    : true,
                               failIfNotNew         : true,
                               pattern              : '**/*valgrind.xml',
                               skipNoTestFiles      : true,
                               stopProcessingIfError: true]
                      ]
                ])
                warnings canComputeNew: false, canResolveRelativePaths: false, consoleParsers: [
                        [parserName: 'GNU C Compiler 4 (gcc)']
                ], healthy: '0', unstableTotalAll: '0', excludePattern: '.*/include/gmock/.*,.*/include/gtest/.*'
            }
        }
    }
}
emailext body: '${JELLY_SCRIPT,template="html"}', mimeType: 'text/html', subject: 'DVJSON', to: 'david@vanlaatum.id.au'
