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

**Txapelketaren formatua editatu** leihoan txapelketaren izena eta proba guztiak interfaze grafikoan konfiguratzen dira: iraupena, puntuatzeko irizpideak eta lehentasuna, sailkapen metatua, zenbat jokalari pasatzen diren eta berdinketen tratamendua. Probak gehitu, kendu edo ordena daitezke. JSON botoiak formatua inportatu edo esportatzeko dira; konfigurazioa ez da testu-editore batean idazten. Formatua aplikatzeak uneko jokalariak eta emaitzak berrabiarazten ditu, eta aurretik berrespena erakusten du.

Adibide bat dago [data/txapelketa_formatua.json](data/txapelketa_formatua.json) fitxategian. Inportatutako JSONa GUIko kontrolen bidez berrikusi eta aldatu daiteke aplikatu aurretik.

JSONen arauak:

- `rounds` zerrendak proben kopurua eta ordena definitzen ditu. Proba bakoitzak `durationMinutes` eta `criteria` ditu; `id` eta `name` aukerakoak dira. Izenik ez badago, aplikazioak proba-zenbakia erabiliko du.
- `criteria` zerrendak `time`, `errors` edo biak onartzen ditu. Ordenak sailkapen-lehentasuna adierazten du; `errors` lehenetsita badago, akats gutxien duen jokalaria doa aurretik.
- `advanceCount` proba batetik hurrengora zenbat jokalari igarotzen diren da. `null` balioak denek aurrera egiten dutela esan nahi du. Azken proban ez da eremu hori jartzen.
- `rankingScope` balioak `round` (proba horretako emaitzak) edo `cumulative` (aurreko probetako metatutako emaitzak) izan daitezke. Proba horretako `criteria` ordena erabiltzen da metaketa alderatzeko.
- `tiePolicy` balioak `includeTies` (muga berdinduta badago, berdindutako guztiak pasatzen dira) edo `manual` (antolatzaileak aukeratzen ditu) izan daitezke.
- `schemaVersion` balioak `1` izan behar du. Proba bakoitzak gutxienez irizpide bat izan behar du, eta iraupena 1 eta 1440 minutu artekoa da.

Probaz aldatzeko, ez da beharrezkoa jokalari guztien emaitzak osatzea. Osatutako emaitzak osatugabeen aurretik sailkatzen dira; ebaketa-kopuruak osatugabeak ere hartzen baditu, aurrera pasa daitezke. Itxitako probako sailkapen- eta aurrerapen-erabakiak saioan gordetzen dira.

Jokalarientzako kopuru zehatza ezartzen duen ebaketa bakoitzaren ondoren, taulak ebaketa horretara arteko akats metatuak eta denbora metatua erakusten ditu, sartutako emaitzekin berriro kalkulatuta. Emaitzaren bat falta bada, metatua eskuragarri dauden balioekin kalkulatzen da. Denek aurrera egiten duten probetan ez da ebaketa-zutaberik gehitzen.

Taulak proba bakoitzean konfiguratutako irizpideei dagozkien emaitza-zutabeak erakusten ditu; denbora hautatuta ez badago, ez da denbora-zutaberik agertzen. Denborak MM:SS formatuan erakusten eta editatzen dira. Edozein probatako eta jokalaritako emaitza-gelaxkak edita daitezke; sailkapena automatikoki eguneratzen da. Uneko proba goiko hautatzailean aukera daiteke. Aurreko proba hautatzean, ondorengo probetako denbora eta akatsak gordetzen dira. Aurrera berriro egiten denean, hurrengo probaren sailkapena berriro kalkulatzen da, gordetako emaitzak mantenduta. Sailkapenean proba horretarako formatuan hautatutako irizpideak bakarrik hartzen dira kontuan.

## Emaitzak gorde eta esportatzea

**Emaitzak gorde/ireki** aukerek JSON saio-fitxategia erabiltzen dute. Fitxategiak formatua, emaitzak, uneko proba eta izoztutako sailkapenak gordetzen ditu. CSV esportazioak taula irakurtzeko balio du, baina ez du formatua edo aurrerapen-egoera gordetzen. Aurreko 9 eta 10 zutabeko CSV saioak ireki daitezke.

Aplikazioak azken saioaren JSON kopia automatikoa gordetzen du erabiltzailearen aplikazio-datuen karpetan, eta hurrengo abiaraztean automatikoki kargatzen du. Aurreko CSV kopia badago, hura ere berreskura daiteke.
