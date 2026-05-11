{
  uses_user_defaults: true,
  security_policy_supported_versions: { '0.0.x': ':white_check_mark:' },
  project_name: 'hwview',
  local _self = self,
  version: '0.0.3',
  description: 'Hardware information utility, inspired by Redmond.',
  custom_project_badges: [
    {
      anchor: '[![Tests](https://github.com/Tatsh/hwview/actions/workflows/tests.yml/badge.svg)]',
      href: 'https://github.com/Tatsh/hwview/actions/workflows/tests.yml',
    },
    {
      anchor: '[![Coverage Status](https://coveralls.io/repos/github/Tatsh/hwview/badge.svg?branch=master)]',
      href: 'https://coveralls.io/github/Tatsh/hwview?branch=master',
    },
  ],
  keywords: ['device', 'hardware', 'viewer', 'system', 'information', 'linux'],
  want_main: false,
  want_codeql: false,
  want_tests: false,
  package_json+: {
    cspell+: {
      ignorePaths+: [
        '.docs/*.tags',
        '.docs/*.tag.xml',
      ],
    },
    scripts+: {
      'flatpak-build-install': 'flatpak run --command=flathub-build org.flatpak.Builder --install sh.tat.%s.yml' % _self.project_name,
      'flatpak-install': 'flatpak uninstall -y  || true && flatpak install -y --user --reinstall flathub sh.tat.%s' % _self.project_name,
      'flatpak-lint': 'flatpak run --command=flatpak-builder-lint org.flatpak.Builder manifest sh.%s.hwview.yml' % _self.project_name,
      'flatpak-run': 'flatpak run sh.tat.%s' % _self.project_name,
      'flatpak-uninstall': 'flatpak uninstall -y sh.tat.%s' % _self.project_name,
    },
  },
  prettierignore+: ['*.desktop', '*.dmexport', '*.tags', 'src/icon.rc'],
  cz+: {
    commitizen+: {
      version_files+: [
        'CMakeLists.txt',
        'man/hwview.1',
        'sh.tat.hwview.yml',
        'snapcraft.yaml',
        'src/main.cpp',
      ],
    },
  },
  shared_ignore+: [
    '/.flatpak-builder/',
    '/build_fp/',
    '/repo/',
  ],
  vscode+: {
    c_cpp+: {
      configurations: [
        {
          cStandard: 'c23',
          compilerPath: '/usr/bin/gcc',
          cppStandard: 'c++23',
          includePath: [
            '${workspaceFolder}/src/**',
            '${workspaceFolder}/build/**/include',
          ],
          name: 'Linux',
        },
      ],
    },
    settings+: {
      'files.associations': {
        '*.moc': 'cpp',
        '*.ui': 'xml',
        'i18n/*.ts': 'xml',
      },
    },
  },
  // C++ only
  cmake+: {
    uses_qt: true,
  },
  project_type: 'c++',
  vcpkg+: {
    dependencies: [
      {
        name: 'ecm',
        'version>=': '6.7.0',
      },
      {
        features: ['concurrent', 'gui', 'network', 'widgets'],
        name: 'qtbase',
        'version>=': '6.8.3',
      },
    ],
  },
  github+: {
    publish_winget: {
      identifier: 'Tatsh.',
      max_versions_to_keep: 1,
    },
  },
}
