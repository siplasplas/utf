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
for locale in root pl cs de ru uk; do
    echo "  - ${locale}.xml"
    wget -q -nc -O "cldr/collation/${locale}.xml" \
        "https://raw.githubusercontent.com/unicode-org/cldr/main/common/collation/${locale}.xml" || true
done

echo "Gotowe. Dane zapisane w: ${DATA_DIR}"
ls -la
