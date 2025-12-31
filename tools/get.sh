#!/bin/bash
# Pobiera dane Unicode - NIE dodawać do repo
# Uruchom: ./tools/get.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DATA_DIR="${SCRIPT_DIR}/unicode_data"

mkdir -p "${DATA_DIR}"
cd "${DATA_DIR}"

echo "Pobieranie danych UCD (Unicode Character Database)..."
wget -q -nc https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt || true
wget -q -nc https://www.unicode.org/Public/UCD/latest/ucd/SpecialCasing.txt || true
wget -q -nc https://www.unicode.org/Public/UCD/latest/ucd/CaseFolding.txt || true

echo "Pobieranie danych CLDR collation..."
mkdir -p cldr/collation

# Wszystkie dostępne locale z CLDR collation
# Lista z: https://github.com/unicode-org/cldr/tree/main/common/collation
LOCALES="
root
af am ar ar_SA as az be bg bn bo bs ca ceb chr ckb cs cy da de de_AT
dsb ee el en en_US_POSIX eo es es_419 et fa fa_AF fi fil fo fr
ga gl gu ha haw he hi hr hsb hu hy id ig is it ja ka kk kl km kn
ko kok ku ky lb lo lt lv mk ml mn mr ms mt my nb ne nl nn no
om or pa pa_Guru pl ps pt ro ru se si sk sl smn sq sr sr_Latn
sv sw ta te th ti tk to tr ug uk ur uz vi vo wae wo xh yi yo
zh zh_Hans zh_Hant zh_Hant_HK zu
"

for locale in $LOCALES; do
    if [ -n "$locale" ]; then
        echo "  - ${locale}.xml"
        wget -q -nc -O "cldr/collation/${locale}.xml" \
            "https://raw.githubusercontent.com/unicode-org/cldr/main/common/collation/${locale}.xml" 2>/dev/null || rm -f "cldr/collation/${locale}.xml"
    fi
done

# Usuń puste pliki (nie wszystkie locale mają pliki collation)
find cldr/collation -empty -delete 2>/dev/null || true
echo "  Pobrano plików: $(ls cldr/collation/*.xml 2>/dev/null | wc -l)"

echo "Gotowe. Dane zapisane w: ${DATA_DIR}"
ls -la
