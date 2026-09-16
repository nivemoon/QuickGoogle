$ErrorActionPreference = 'Stop'

$appName = 'GoogleSearchContext'
$installDir = Join-Path $env:LOCALAPPDATA $appName
$dllName = 'GoogleSearchContext.dll'
$sourceDll = Join-Path $PSScriptRoot $dllName
$targetDll = Join-Path $installDir $dllName
$clsid = '{9D7E5F81-26A4-4F32-916D-438DA1725B09}'

New-Item -ItemType Directory -Path $installDir -Force | Out-Null
Copy-Item -LiteralPath $sourceDll -Destination $targetDll -Force

$classes = 'HKCU:\Software\Classes'
$handlerKey = "$classes\AllFilesystemObjects\shellex\ContextMenuHandlers\GoogleSearchContext"
$clsidKey = "$classes\CLSID\$clsid"
$serverKey = "$clsidKey\InprocServer32"

New-Item -Path $handlerKey -Force | Out-Null
Set-ItemProperty -Path $handlerKey -Name '(default)' -Value $clsid

New-Item -Path $serverKey -Force | Out-Null
Set-ItemProperty -Path $clsidKey -Name '(default)' -Value 'Google Search Context Menu'
Set-ItemProperty -Path $serverKey -Name '(default)' -Value $targetDll
Set-ItemProperty -Path $serverKey -Name 'ThreadingModel' -Value 'Apartment'

Add-Type @'
using System;
using System.Runtime.InteropServices;

public static class ShellRefresh {
    [DllImport("shell32.dll")]
    public static extern void SHChangeNotify(
        uint wEventId,
        uint uFlags,
        IntPtr dwItem1,
        IntPtr dwItem2
    );
}
'@

[ShellRefresh]::SHChangeNotify(0x08000000, 0, [IntPtr]::Zero, [IntPtr]::Zero)

Write-Host "GoogleSearchContext установлен."
Write-Host "Перезапусти Explorer, если пункт не появился."