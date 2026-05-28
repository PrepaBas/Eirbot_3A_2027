# EIRBOT 3A 2027
## Equipe
- Barthere Aurelien E3
- Bromet Alexandre E3
- Daragnes Le"ia I3
- Guigui Marceau E3
- Halimi Zakaria E3
- Louvel Elise E3
- Marie Quentin E3
- Tran-Ruesche Bastien E3

#### Traquenard
- Berthod Hugo E3
- Donias Arno I3
- Michaud Lou-Ann E3

#### Refus
- 

## Rôles 
*Scrum Master* -> Bastien  
*Respo Base Méca* -> Aurelien  
*Respo*

## Esthétique du robot
Un robot au couleurs du Ara bleu. Des dimensions modestes (carré de 25 cm), deux espaces distincts en bas et en haut. Une tour excentrée pour relier ces deux espaces. Les actionneurs comblent le reste.  

## Méca
### Structure
Une plaque de 5mm (acrilique, aluminium ou acier) de 250x250mm avec les coins coupés à 5mm du sol. une bille principale à l'arrière et des patins de protection partout autour. (A voir pour une plaque de 240x240mm mais en rajoutant du flexible en protection).  
Des makerbeam (extrusion aluminium de 10x10mm) s'attachent à la plaque horizontalement et une tour de quatre extrusions s'élève jusqu'à h=30mm. En haut, d'autres extrusions forment une base pour monter l'électronique, un écran, une balise, l'aru...

### Bloc moteur
Stepper Nema 17 avec encodeuse As5047p. Contrôlé (FOC) par ST B-g431Esc suivant le modèle HybridStepper de SimpleFOC.  
Roue encodeuse sur table suspendue par deux rails linéaires MGN9H, MGN12C ou MGN12H. Roues sur deux roulements et encodée avec le capteur magnétique As5047p encore une fois. Force linéaire descendante avec des élastiques de bureau.  
Cet ensemble forme un cylindre qui s'attache à la plaque du dessous. Les vis sont fraisées et montée par le dessous. L'alignement est à voir. 
Roues de romba ou tube silicon ou silicon de cuisine...

### Matériaux
Du plastique imprimé en 3D / de l'acrylique découpé à la laser et eventuellement du métal si on a les moyens vers la fin.
Des makerbeam pour tout le reste du structurel.

### Visserie
Standart vis CHC de 3 ou 4. Préferer la 10, la 16 et la 20.  
Vis FHC pour viser depuis la plaque du bas.  
Maximiser les écrous pour l'assemblage, ou les inserts filletés en moindre mesure.  
Ecrous frein ou Loctite si besoin.
Pas de cruciforme, pas de tête plate, pas de tête bombée, pas de bois, pas 
Le press-fit ca marche bien, la colle ca peut bien marcher aussi.

## Elec
### Puissance
J'ai deux batteries lipo 4s (15V) mais ca va être compliqué de mettre les deux sur le robot parce qu'elles sont grosses. Donc robot en 12V à part si on trouve une solution pour caser les deux / racheter des batteries plus petites.  
prévoir 
- 5.2V 5A en USB-C pour la rasp
- 2x 12V 2.5A pour les moteurs (et pas mal de réserve en pic)
- rails 12V et 5V pour actionneurs et capteurs

##### Et eventuellement
- 5V 0.5A en USB-C pour la STm
- 

#### Connectique
XT-30 : femelle donne et mâle reçoit.

#### Protection
Fusible au moins mais je ne m'y connais pas

### Logique
Acheter des fils petits c'est beaucoup mieux. JST 2.5mm c'est pour l'instant le plus pratique

### Ventilation
Ce serai bien d'en mettre

## Info
### Rasp
#### Ecran
Ecran tactile si possible avec une interface graphique (html par exemple) qui permet d'executer des scripts.

#### ROS2
Framwork Ros2 dans un docker pour pouvoir laisser raspos en natif. Le but est de commencer à écrire nos propres nodes. La strat générale c'est d'utiliser les Behavior Tree pour séquencer les actions : il faut donc faire des serveurs
- rotation
- ligne droite
- path planning sommaire pour éviter nav2
- actionneurs

Il faut aussi faire la détection et tout enfin bref une belle galère.  
Exit gazebo vive rviz tout court  

EKF pour les roues odometriques, l'imu et autre si affinité.

Exit le controller differentiel, ca se fera sur mcu.

#### Behavior Tree
Permet de rendre le robot dynamique. Par exemple, recherche d'un goal, séquence pour arriver au goal, maneuvre et actionneurs et rebelotte. Si un obstacle est détecté, on peut attendre et réessayer avant de revenir au choix de l'action.

Dans l'idéal il y a un serveur en python (rasp) et un serveur en c++ (mcu) pour distinguer la simu du réel.

### MCU
STM32 mp1814C qui embarque un st32h533re bien balèze. Host micro-ros pour communiquer avec le ros2 de la rasp. 
Programmation hors framework Arduino donc avec la synthaxe ST et la configuration sur CubeMX. L'utilisation de micro-ros 'impose' une toolchain Makefile.  
Le MCU doit gérer : 
- Envoi des commandes CAN de vitesse aux ESC
- Reception des données encodeuses. Signaux AB récupérés avec les timers en mode encoder.
- Asserv rapide et capacité de tourner, avancer (voir se déplacer dans le plan mais on est pas encore là).
- Récupération via SPI de l'IMU **ICM42688** et envoi direct vers la rasp.
- Récupération des données TOF si il y a
- Gestion des timeout, de l'évitement de poing, du timer de 100 secondes : il faut donc plusieurs modes pour le debug et le match (pourquoi pas un topic)
- des serveurs pour tourner, avancer et faire marcher les actionneurs.

## Evitement
Un lidar si on veut branché sur la rasp et qui permet de faire de l'évitement longue distance  
Des tof branchés au MCU pour la défense de poing.

Dans le meilleur des mondes des zones (devant près, devant loin, droite près ...) pour éviter de tourner avec les actionneurs déployés.