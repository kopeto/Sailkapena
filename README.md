# SailkapenaHG

Lehiaketako parte-hartzaileak, denborak, akatsak eta sailkapena kudeatzeko Qt 6 aplikazioa.

## Linuxen konpilatzea

Ubuntu edo Debianen, `build.sh` scriptak behar diren paketeak instalatu eta Release bertsioa konpilatzen du:

```bash
./build.sh
./build/SailkapenaHG
```

Eskuz konpilatzeko, instalatu `build-essential`, `cmake` eta `qt6-base-dev` paketeak, eta exekutatu:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/SailkapenaHG
```

## Windowsen konpilatzea

Instalatu Qt 6, CMake, C++ konpilatzailea eta, instalatzailea sortu nahi bada, Inno Setup. `build_windows.bat` scriptak Qt-en instalazioa bilatzen du `C:\Qt` karpetan. Bestelako kokapena bada, ezarri `QT_DIR` scriptaren hasieran. Ondoren exekutatu:

```bat
build_windows.bat
```

## Sailkapen-arauak

- Lehen bi probetan, proba gehiago osatu dituzten parte-hartzaileak lehenago agertzen dira.
- Berdinketarik badago, akats gutxien dituenak eta gero denbora txikiena duenak egiten du gora.
- Berdinketak jarraituz gero, jokalarien izenak euskarazko ordena localean alderatzen dira.
- Lehen sei sailkatuak finalistak dira. Finalean, denbora erregistratua duten finalistak lehenago agertzen dira; ondoren, akats eta denbora gutxien dituztenak.
- Proba-hautagailuak denbora zein probatan erregistratuko den aukeratzen du. Sailkapena eguneratzean, lehen bi probetako emaitza guztiak hartzen dira kontuan.
- Emaitza berriak sartu ondoren, sakatu **Sailkapena eguneratu** sailkapen-ordena berrikusteko.

## Emaitzak gordetzea

Emaitzak UTF-8 kodetutako eta puntu eta koma bidez bereizitako CSV fitxategi batean gordetzen dira. Fitxategiak goiburua dauka; aplikazioak formatu hori eta lehendik erabilitako 9 eta 10 zutabeko bi proba eta final formatuak ireki ditzake. Hiru probako artxiboen denborak guztizkoarekin bat datozela antzematen bada, fitxategia ez da kargatzen, emaitzak proba oker bati esleitu ez dakizkion. Izenetan `;` edo komatxoak badaude, CSV formatuan ihes egiten dira.

Aplikazioak azken emaitzen kopia automatikoa gordetzen du erabiltzailearen aplikazio-datuen karpetan. Hurrengo abiaraztean aurreko saioa berreskuratu nahi den galdetzen du.
