# Script pour ouvrir l'interface web
$path = "C:\Users\Ikram\OneDrive\Desktop\MorphoArabe\interface.html"

Write-Host "===================================" -ForegroundColor Cyan
Write-Host "محرك الصرف العربي - واجهة التجربة" -ForegroundColor Yellow
Write-Host "===================================" -ForegroundColor Cyan
Write-Host ""

if (Test-Path $path) {
    Write-Host "✓ فتح الواجهة..." -ForegroundColor Green
    Start-Process $path
    Write-Host "✓ يجب أن ينفتح المتصفح الافتراضي" -ForegroundColor Green
    Write-Host ""
    Write-Host "نصائح الاختبار:" -ForegroundColor Yellow
    Write-Host "1. جرب تحميل من الملف"
    Write-Host "2. أضف جذور جديدة"
    Write-Host "3. جرب توليد المشتقات"
    Write-Host "4. اختبر الأفعال الجوفاء (أجوف)"
} else {
    Write-Host "✗ لم يتم العثور على الملف!" -ForegroundColor Red
    Write-Host "المسار: $path" -ForegroundColor Red
}

Write-Host ""
