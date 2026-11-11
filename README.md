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

## Txapelketaren formatua

**Txapelketaren formatua editatu** leihoan txapelketaren izena eta txanda guztiak interfaze grafikoan konfiguratzen dira: iraupena, puntuatzeko irizpideak eta lehentasuna, sailkapen metatua, zenbat jokalari pasatzen diren eta berdinketen tratamendua. Txandak gehitu, kendu edo ordena daitezke. JSON botoiak formatua inportatu edo esportatzeko dira; konfigurazioa ez da testu-editore batean idazten. Formatua aplikatzeak uneko jokalariak eta emaitzak berrabiarazten ditu, eta aurretik berrespena erakusten du.

Adibide bat dago [data/txapelketa_formatua.json](data/txapelketa_formatua.json) fitxategian. Inportatutako JSONa GUIko kontrolen bidez berrikusi eta aldatu daiteke aplikatu aurretik.

JSONen arauak:

- `rounds` zerrendak txanden kopurua eta ordena definitzen ditu. Txanda bakoitzak `durationMinutes` eta `criteria` ditu; `id` eta `name` aukerakoak dira. Izenik ez badago, aplikazioak txanda-zenbakia erabiliko du.
- `criteria` zerrendak `time`, `errors` edo biak onartzen ditu. Ordenak sailkapen-lehentasuna adierazten du; `errors` lehenetsita badago, akats gutxien duen jokalaria doa aurretik.
- `advanceCount` txanda batetik hurrengora zenbat jokalari igarotzen diren da. `null` balioak denek aurrera egiten dutela esan nahi du. Azken txandan ez da eremu hori jartzen.
- `rankingScope` balioak `round` (txanda horretako emaitzak) edo `cumulative` (aurreko txandetako metatutako emaitzak) izan daitezke. Txanda horretako `criteria` ordena erabiltzen da metaketa alderatzeko.
- `tiePolicy` balioak `includeTies` (muga berdinduta badago, berdindutako guztiak pasatzen dira) edo `manual` (antolatzaileak aukeratzen ditu) izan daitezke.
- `schemaVersion` balioak `1` izan behar du. Txanda bakoitzak gutxienez irizpide bat izan behar du, eta iraupena 1 eta 1440 minutu artekoa da.

Txanda ixteko, parte-hartzaile hautagarri guztien irizpide-emaitzak osatu behar dira. Itxitako txandako emaitzak eta hurrengoan pasatutako jokalariak saioan gordetzen dira; emaitzak ez dira automatikoki berriz sailkatzen geroagoko txandetan.

Jokalarientzako kopuru zehatza ezartzen duen ebaketa bakoitzaren ondoren, taulak ebaketa horretara arteko denbora eta akats metatuak erakusten ditu. Denek aurrera egiten duten txandetan ez da ebaketa-zutaberik gehitzen.

Taulak txanda bakoitzean konfiguratutako irizpideei dagozkien emaitza-zutabeak erakusten ditu; denbora hautatuta ez badago, ez da denbora-zutaberik agertzen. Uneko txanda goiko hautatzailean aukera daiteke; hurrengo txanda hautatzeak oraingoa ixten du, emaitza guztiak osatuta badaude. Aurreko txanda berrirekitzean, ondorengo txandetako emaitzak eta sailkapenak ezabatzen dira, berrespena eman ondoren. Sailkapenean txanda horretarako JSON formatuan hautatutako irizpideak bakarrik hartzen dira kontuan.

## Emaitzak gorde eta esportatzea

**Emaitzak gorde/ireki** aukerek JSON saio-fitxategia erabiltzen dute. Fitxategiak formatua, emaitzak, uneko txanda eta izoztutako sailkapenak gordetzen ditu. CSV esportazioak taula irakurtzeko balio du, baina ez du formatua edo aurrerapen-egoera gordetzen. Aurreko 9 eta 10 zutabeko CSV saioak ireki daitezke.

Aplikazioak azken saioaren JSON kopia automatikoa gordetzen du erabiltzailearen aplikazio-datuen karpetan, eta hurrengo abiaraztean automatikoki kargatzen du. Aurreko CSV kopia badago, hura ere berreskura daiteke.
