# Script de test pour les verbes creux (Ajwaf)

Write-Host "Test des verbes creux (Ajwaf)"
Write-Host "=============================="

$input_commands = @"
1
12
a
1
0
0
"@

# Exécuter le programme avec les entrées
$input_commands | & ".\moteur.exe"
