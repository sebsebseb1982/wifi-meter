# wifi-meter

Mini détecteur de force du signal WiFi à base d'un **ESP32-WROOM-32D** monté sur le PCB du projet `game-of-life` : écran **OLED SSD1306 128×64 0,96"** (I2C), **3 boutons**, **capteur de luminosité** et LED du PCB.

- Connexion au réseau WiFi choisi via un **portail captif** (cf. ci-dessous), mémorisé dans la flash
- 4 pages, navigation avec les boutons :

| Bouton    | Action                                                            |
|-----------|-------------------------------------------------------------------|
| Gauche    | Page précédente                                                   |
| Droite    | Page suivante                                                     |
| Milieu    | Action de la page (cf. ci-dessous)                                |
| Milieu (appui long, 3 s) | Ouvrir le portail de configuration WiFi           |

| Page           | Contenu                                                                                 | Bouton milieu                     |
|----------------|-----------------------------------------------------------------------------------------|-----------------------------------|
| Signal         | SSID, canal, IP, RSSI lissé (dBm), 5 barres, qualité en %, historique ~32 s             | Couper / réactiver le clignotant  |
| Historique     | Min / max / moyenne du RSSI et courbe pleine page                                       | Remettre les stats à zéro         |
| Infos          | IP, passerelle, MAC, BSSID du point d'accès, uptime, nb de connexions, luminosité       | -                                 |
| Réseaux        | Les 6 réseaux les plus puissants à proximité (SSID, canal, RSSI), `*` = réseau connecté | Relancer un scan                  |

- Qualité en % : -100 dBm = 0 %, -50 dBm = 100 %. Une mesure toutes les 250 ms.
- La **LED** du PCB (GPIO 17) clignote d'autant plus vite que le signal est fort (de 1,5 s à 80 ms entre deux éclairs). Elle reste éteinte tant que la carte n'est pas connectée.
- Le **capteur de luminosité** règle automatiquement le contraste de l'écran (plus sombre dans le noir).

```
+---------------------------+
|MaBox-WiFi        ▪ □ □ □  |
|ch6 192.168.1.42           |
|-58 dBm           . | | | ||
|[#############    ]  84%   |
|  ~~~~\__/~~~~~~\_/~~~~~~  |
+---------------------------+
```

## Configuration du WiFi (portail captif)

Il n'y a plus de mot de passe dans le code : le réseau se choisit depuis un téléphone ou un PC.

1. Le portail s'ouvre automatiquement au premier démarrage, ou après 3 échecs de connexion (3 × 15 s). On peut aussi l'ouvrir à tout moment par un **appui long de 3 s sur le bouton du milieu**.
2. L'écran affiche la marche à suivre et la LED clignote lentement. Se connecter au réseau WiFi **`WiFi-Meter`** (ouvert par défaut).
3. Le navigateur propose normalement de lui-même la page de connexion. Sinon, ouvrir `http://192.168.4.1`.
4. Choisir un réseau dans la liste (ou saisir un SSID à la main), entrer le mot de passe, valider.
5. Le WiFi meter enregistre le réseau, redémarre et s'y connecte.

Si personne ne se connecte au portail et qu'un réseau est déjà enregistré, le portail se ferme au bout de 5 minutes et la connexion au réseau enregistré est retentée.

Le nom et le mot de passe du point d'accès se règlent dans [wifi-portal.h](include/wifi-portal.h). Avec le point d'accès ouvert, le mot de passe saisi circule en clair entre le téléphone et la carte : définir `WIFI_PORTAL_AP_PASSWORD` (8 caractères minimum) si cela pose problème.

## Configuration

Les réglages se trouvent dans les headers :

| Fichier                                        | Réglage                                                                      |
|------------------------------------------------|------------------------------------------------------------------------------|
| [pinout.h](include/pinout.h)                   | Broches de l'écran, des boutons, du capteur de lumière et de la LED          |
| [wifi-portal.h](include/wifi-portal.h)         | Nom / mot de passe du point d'accès du portail                               |
| [wifi-connection.h](include/wifi-connection.h) | Nombre d'essais avant le portail, délai de fermeture du portail              |
| [status-led.h](include/status-led.h)           | Durée et périodes de clignotement                                            |
| [signal-strength.h](include/signal-strength.h) | Période d'échantillonnage et lissage du RSSI                                 |
| [light-sensor.h](include/light-sensor.h)       | Lissage et sens du capteur (`LIGHT_SENSOR_INVERTED`)                         |
| [screen.h](include/screen.h)                   | Plage de contraste de l'écran                                                |
| [debug.h](include/debug.h)                     | Activer les traces sur le port série (`DEBUG_SERIAL`)                        |

## Câblage

Tout est déjà câblé sur le PCB, rien à ajouter. Brochage déduit du sketch `game-of-life` (cf. [pinout.h](include/pinout.h)) :

| Périphérique        | GPIO | Remarque                                          |
|---------------------|------|---------------------------------------------------|
| OLED SDA / SCL      | 21 / 22 | I2C par défaut, adresse `0x3C`                 |
| Bouton gauche       | 32   | Actif à l'état haut                               |
| Bouton milieu       | 33   | Actif à l'état haut                               |
| Bouton droit        | 25   | Actif à l'état haut                               |
| Capteur de lumière  | 26   | ADC2, cf. ci-dessous                              |
| LED                 | 17   | Allumée à l'état haut                             |

> **ADC2 et WiFi** : le GPIO 26 est sur l'ADC2, que le pilote WiFi de l'ESP32 utilise aussi. Selon la version du core Arduino, la lecture peut échouer (valeur 0) tant que le WiFi est actif. Si c'est le cas, la page Infos affiche une luminosité fixe et l'écran garde son contraste maximum. La seule solution matérielle est de déplacer le capteur sur une broche ADC1 (GPIO 32 à 39).

## Flasher la carte

La carte se branche en USB (pont CP210x, port COM5 sur ce PC, cf. `upload_port` dans [platformio.ini](platformio.ini)).

Cette carte n'a pas d'auto-reset : si l'upload reste bloqué sur `Connecting...` puis échoue avec `Wrong boot mode detected`, maintenir le bouton **BOOT** de l'ESP32 pendant toute la phase `Connecting...`, et le relâcher dès que l'écriture commence. Appuyer ensuite sur **EN** pour démarrer le programme.

```sh
pio run                            # compiler
pio run -t upload                  # compiler et flasher
pio run -t upload --upload-port COM5
pio device monitor                 # moniteur série (115200 bauds)
```

Si l'upload échoue à 460 800 bauds (câble long), baisser `upload_speed` à `115200`.

`monitor_rts` et `monitor_dtr` sont forcés à 0 dans [platformio.ini](platformio.ini) : sinon le moniteur série maintient la carte en reset ou en mode programmation.

## Dépannage

| Symptôme                                   | Piste                                                                              |
|--------------------------------------------|------------------------------------------------------------------------------------|
| Écran noir                                 | Vérifier l'adresse I2C (`0x3C` ou `0x3D`)                                          |
| `SSID introuvable` à l'écran               | SSID erroné, ou réseau en 5 GHz uniquement (l'ESP32 ne gère que le 2,4 GHz)        |
| `Echec connexion`                          | Mot de passe erroné : après 3 essais le portail s'ouvre pour le corriger           |
| Pas de popup de connexion sur le téléphone | Ouvrir `http://192.168.4.1` à la main (désactiver les données mobiles)             |
| Boutons inversés / sans effet              | Vérifier les broches dans [pinout.h](include/pinout.h)                             |
| Écran trop sombre en pleine lumière        | Passer `LIGHT_SENSOR_INVERTED` à `1`                                               |
| `Wrong boot mode detected` à l'upload      | Maintenir BOOT pendant `Connecting...`                                             |
