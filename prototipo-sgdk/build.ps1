param([string]$Sgdk = $env:GDK)

$ErrorActionPreference = 'Stop'
if (-not $Sgdk) {
    throw 'Informe o caminho do SGDK com -Sgdk ou defina a variavel GDK.'
}

$sdkPath = (Resolve-Path -LiteralPath $Sgdk).Path
$make = Join-Path $sdkPath 'bin/make.exe'
$makefile = Join-Path $sdkPath 'makefile.gen'
if (!(Test-Path -LiteralPath $make -PathType Leaf) -or !(Test-Path -LiteralPath $makefile -PathType Leaf)) {
    throw 'O caminho informado precisa conter bin/make.exe e makefile.gen do SGDK.'
}

Push-Location -LiteralPath $PSScriptRoot
try {
    & $make -f $makefile release
    if ($LASTEXITCODE -ne 0) { throw "A compilacao falhou (make $LASTEXITCODE)." }
    Write-Output (Join-Path $PSScriptRoot 'out/rom.bin')
} finally {
    Pop-Location
}
