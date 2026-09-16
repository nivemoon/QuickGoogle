$ErrorActionPreference = 'SilentlyContinue'

$appName = 'GoogleSearchContext'
$installDir = Join-Path $env:LOCALAPPDATA $appName
$classes = 'HKCU:\Software\Classes'
$clsid = '{9D7E5F81-26A4-4F32-916D-438DA1725B09}'

Remove-Item `
    -LiteralPath "$classes\AllFilesystemObjects\shellex\ContextMenuHandlers\GoogleSearchContext" `
    -Recurse `
    -Force

Remove-Item `
    -LiteralPath "$classes\CLSID\$clsid" `
    -Recurse `
    -Force

Remove-Item `
    -LiteralPath $installDir `
    -Recurse `
    -Force

Write-Host "GoogleSearchContext удалён."
Write-Host "Перезапусти Explorer."