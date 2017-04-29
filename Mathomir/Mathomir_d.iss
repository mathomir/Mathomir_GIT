; -- Mathomir.iss --
[Setup]
AppName=Math-o-mir
AppVerName=Math-o-mir v2.0
DefaultDirName={pf}\Mathomir
DefaultGroupName=Math-o-mir
UninstallDisplayIcon={app}\Mathomir.exe
Compression=lzma
SolidCompression=yes
OutputDir=C:\Mathomir\Setup
ChangesAssociations=yes

[Registry]
Root: HKCR; Subkey: ".mom"; ValueType: string; ValueName: ""; ValueData: "MathomirFile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "MathomirFile"; ValueType: string; ValueName: ""; ValueData: "Math-o-mir file"; Flags: uninsdeletekey
;Root: HKCR; Subkey: "MathomirFile\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\Mathomir.exe,0"
Root: HKCR; Subkey: "MathomirFile\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\Mathomir.exe"" ""%1"""

[InstallDelete]
Type: files; Name: "{app}\Language.csv"
Type: files; Name: "{app}\HandyHelp.chm"
Type: files; Name: "{app}\HandyHelp.pdf"
Type: files; Name: "{app}\FirstRun_.mom"

[UninstallDelete]
Type: files; Name: "{app}\Mathomir.chm"
Type: files; Name: "{app}\FirstRun_.mom"

[Files]
Source: "..\Release\Mathomir.exe"; DestDir: "{app}"
Source: "Example.mom"; DestDir: "{app}"
Source: "FirstRun_.mom"; DestDir: "{app}"; Permissions: everyone-modify
;Source: "Readme.txt"; DestDir: "{app}"; Flags: isreadme
Source: "Language_d.csv"; DestDir: "{app}"; DestName: "Language.csv"

[Icons]
Name: "{group}\Math-o-mir"; Filename: "{app}\Mathomir.exe"
Name: {group}\Uninstall Math-o-mir; Filename: {uninstallexe}

[Dirs]
Name: "{app}"; Permissions: everyone-modify