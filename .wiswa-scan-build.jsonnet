{
  checkout_options: { 'fetch-depth': 0 },
  configure_command: 'cmake -S . -B build-scan -DCMAKE_BUILD_TYPE=Debug',
  documentation_steps: [
    {
      name: 'Install dependencies',
      run: 'sudo apt-get update\nsudo apt-get install -y doxygen graphviz\n',
    },
    {
      env: {
        WORKSPACE: '${{ github.workspace }}',
      },
      name: 'Configure',
      run: 'cmake -DCMAKE_BUILD_TYPE=Release -B "$WORKSPACE/build" --install-prefix "$WORKSPACE/build/prefix" -DBUILD_DOCS_ONLY=ON',
    },
    {
      env: {
        WORKSPACE: '${{ github.workspace }}',
      },
      name: 'Build documentation',
      run: 'cmake --build "$WORKSPACE/build" --verbose --config Release --target docs',
    },
  ],
  env: {},
  setup_steps: [
    {
      name: 'Install build dependencies',
      run: 'sudo apt-get update\nsudo apt-get install -y extra-cmake-modules libudev-dev\n',
    },
    {
      name: 'Install current CMake',
      run: 'pipx install cmake',
    },
    {
      name: 'Install Qt',
      uses: 'jurplel/install-qt-action@a9c63c7c123f3069cff414e7e482d95dfa9d8125',
    },
  ],
  workflow_file: 'pages.yml',
}
