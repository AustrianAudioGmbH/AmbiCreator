#define Version Trim(FileRead(FileOpen("..\VERSION")))
#define ProductName 'AmbiCreator2'
#define Publisher 'AustrianAudio'
#define Year GetDateTimeString("yyyy","","")
#define AC_BUILD_DIR GetEnv('AC_BUILD_DIR')
#define AC_BUILD_ARCHIVE GetEnv('AC_BUILD_ARCHIVE')
#define AC_BUILD_TYPE GetEnv('AC_BUILD_TYPE')

[Setup]
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
AppName={#ProductName}
OutputBaseFilename="{#ProductName}_{#GetEnv('AC_BUILD_MARK')}_Installer"
AppCopyright=Copyright (C) {#Year} {#Publisher}
AppPublisher={#Publisher}
AppVersion={#Version}
DefaultDirName="{commoncf64}\VST3\{#ProductName}.vst3"
DisableDirPage=yes
OutputDir={#AC_BUILD_ARCHIVE}

; MAKE SURE YOU READ THE FOLLOWING!
LicenseFile="EULA"
UninstallFilesDir="{commonappdata}\{#ProductName}\uninstall"

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#ProductName}Data"

; MSVC adds a .ilk when building the plugin. Let's not include that.
[Files]
Source: "{#AC_BUILD_DIR}\AmbiCreator_artefacts\{#AC_BUILD_TYPE}\VST3\{#ProductName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#ProductName}.vst3\"; Excludes: *.ilk; Flags: ignoreversion recursesubdirs;

[Run]
Filename: "{cmd}"; \
    WorkingDir: "{commoncf64}\VST3"; \
    Parameters: "/C mklink /D ""{commoncf64}\VST3\{#ProductName}Data"" ""{commonappdata}\{#ProductName}"""; \
    Flags: runascurrentuser;
