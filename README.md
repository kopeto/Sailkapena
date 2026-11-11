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

Txandaren gaineko **Txapelketaren formatua editatu (JSON)** botoiak formatua aplikaziotik editatzeko aukera ematen du. JSON fitxategia bertan ireki, aldatu, gorde eta aplikatu daiteke. Formatu aldaketak uneko jokalariak eta emaitzak berrabiarazten ditu; aplikazioak berrespena eskatzen du.

Adibide bat dago [data/txapelketa_formatua.json](data/txapelketa_formatua.json) fitxategian. Formatua aplikatzeko, editoreko **JSON kargatu...** botoiarekin ireki eta **Formatua aplikatu** sakatu.

JSONen arauak:

- `rounds` zerrendak txanden kopurua eta ordena definitzen ditu. Txanda bakoitzak `durationMinutes` eta `criteria` ditu; `id` eta `name` aukerakoak dira. Izenik ez badago, aplikazioak txanda-zenbakia erabiliko du.
- `criteria` zerrendak `time`, `errors` edo biak onartzen ditu. Ordenak sailkapen-lehentasuna adierazten du; `errors` lehenetsita badago, akats gutxien duen jokalaria doa aurretik.
- `advanceCount` txanda batetik hurrengora zenbat jokalari igarotzen diren da. `null` balioak denek aurrera egiten dutela esan nahi du. Azken txandan ez da eremu hori jartzen.
- `rankingScope` balioak `round` (txanda horretako emaitzak) edo `cumulative` (aurreko txandetako metatutako emaitzak) izan daitezke. Txanda horretako `criteria` ordena erabiltzen da metaketa alderatzeko.
- `tiePolicy` balioak `includeTies` (muga berdinduta badago, berdindutako guztiak pasatzen dira) edo `manual` (antolatzaileak aukeratzen ditu) izan daitezke.
- `schemaVersion` balioak `1` izan behar du. Txanda bakoitzak gutxienez irizpide bat izan behar du, eta iraupena 1 eta 1440 minutu artekoa da.

Txanda ixteko, parte-hartzaile hautagarri guztien irizpide-emaitzak osatu behar dira. Itxitako txandako emaitzak eta hurrengoan pasatutako jokalariak saioan gordetzen dira; emaitzak ez dira automatikoki berriz sailkatzen geroagoko txandetan.

Sailkapen metatuan, taulak metatutako denbora edo akatsak ere erakusten ditu, sailkapenaren oinarria ikusgai egon dadin.

## Emaitzak gorde eta esportatzea

**Emaitzak gorde/ireki** aukerek JSON saio-fitxategia erabiltzen dute. Fitxategiak formatua, emaitzak, uneko txanda eta izoztutako sailkapenak gordetzen ditu. CSV esportazioak taula irakurtzeko balio du, baina ez du formatua edo aurrerapen-egoera gordetzen. Aurreko 9 eta 10 zutabeko CSV saioak ireki daitezke.

Aplikazioak azken saioaren JSON kopia automatikoa gordetzen du erabiltzailearen aplikazio-datuen karpetan. Aurreko CSV kopia badago, hura ere berreskura daiteke.
