# EIRBOT 3A 2027
## Equipe
### OG
- Barthere Aurelien E3
- Bromet Alexandre E3
- Daragnes Leïa I3
- Guigui Marceau E3
- Halimi Zakaria E3
- Louvel Elise E3
- Marie Quentin E3
- Tran-Ruesche Bastien E3

#### A traquenard
- Berthod Hugo E3
- Donias Arno I3
- Michaud Lou-Ann E3
- Maxime Lavit E3
- Joan Dumarchat I3 (bientôt)

#### Refus
- 

## Compilation micro-ros
checkout to humble then go back to main folder and
``` bash
docker pull microros/micro_ros_static_library_builder:humble
docker run -it --rm -v $(pwd):/project --env MICROROS_LIBRARY_FOLDER=micro_ros_stm32cubemx_utils/microros_static_library microros/micro_ros_static_library_builder:humble
```

## Debug config
debug : Cortex-Debug dans vs code
+ `sudo apt install openocd`
+ fichier `.vscode/launch.json`
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug STM32H533 (micro-ROS)",
            "type": "cortex-debug",
            "request": "launch",
            "servertype": "openocd",
            "cwd": "${workspaceRoot}",
            "executable": "${workspaceRoot}/build/microros_base.elf",
            "device": "STM32H533RE", // Ajuste si la référence exacte de ton H5 change
            "configFiles": [
                "interface/stlink.cfg",
                "target/stm32h5x.cfg"
            ],
            // --- TRÈS IMPORTANT POUR EUROBOT & FREERTOS ---
            "rtos": "FreeRTOS", // Permet à VS Code de voir tes différentes tâches (Odom, micro-ROS...)
            "runToEntryPoint": "main", // S'arrête automatiquement au début du main()
            "openOCDPreConfigLaunchCommands": [
                "set CONNECT_UNDER_RESET 1" // Évite le piège du blocage de la puce !
            ]
        }
    ]
}
```
## Esthétique du robot
???
## Méca
### Structure
Une plaque de 5mm (acrilique, aluminium ou acier) de 250x250mm avec les coins coupés à 5mm du sol. une bille principale à l'arrière et des patins de protection partout autour. (A voir pour une plaque de 240x240mm mais en rajoutant du flexible en protection).  
Des makerbeam (extrusion aluminium de 10x10mm) s'attachent à la plaque horizontalement et une tour de quatre extrusions s'élève jusqu'à h=30mm. En haut, d'autres extrusions forment une base pour monter l'électronique, un écran, une balise, l'aru...

### Bloc moteur
Stepper Nema 17 avec encodeuse As5047p. Contrôlé (FOC) par ST B-g431Esc suivant le modèle HybridStepper de SimpleFOC.  
Roue encodeuse sur table suspendue par deux rails linéaires MGN9H, MGN12C ou MGN12H. Roues sur deux roulements et encodée avec le capteur magnétique As5047p encore une fois. Force linéaire descendante avec des élastiques de bureau.  
Cet ensemble s'attache à la plaque du dessous. Les vis sont fraisées et montée par le dessous. L'alignement est à voir. 
Roues de romba ou tube silicon ou silicon de cuisine...

### Matériaux
Du plastique imprimé en 3D / de l'acrylique découpé à la laser et eventuellement du métal si on a les moyens vers la fin.
Des makerbeam pour tout le reste du structurel.

### Visserie
du chc de préference

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
Acheter des fils petits c'est beaucoup mieux. JST 2.5mm c'est pour l'instant le plus pratique même si c'est gros

### Ventilation
Ce serai bien d'en mettre

## Info
### Rasp
#### Ecran
Ecran tactile si possible avec une interface graphique (html par exemple) qui permet d'executer des scripts.

#### ROS2
Framwork Ros2 dans un docker pour pouvoir laisser raspOS en natif. Le but est de commencer à écrire nos propres nodes. La strat générale c'est d'utiliser les Behavior Tree pour séquencer les actions : il faut donc faire des serveurs
- rotation
- ligne droite
- path planning sommaire pour éviter nav2
- actionneurs

Il faut aussi faire la détection et tout enfin bref une belle galère.  
Exit gazebo vive rviz tout court  

EKF pour les roues odometriques, l'imu et autre si affinité.

Exit le controller differentiel de ros, ca se fera sur mcu.

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
- Récupération via SPI de l'IMU **ICM42688**
- Récupération des données TOF si il y a
- Gestion des timeout, de l'évitement de poing, du timer de 100 secondes : il faut donc plusieurs modes pour le debug et le match (pourquoi pas un topic)
- des serveurs pour tourner, avancer et faire marcher les actionneurs.

## Evitement
Un lidar si on veut, branché sur la rasp et qui permet de faire de l'évitement longue distance  
Des tof branchés au MCU pour la défense de poing.

Dans le meilleur des mondes des zones (devant près, devant loin, droite près ...) pour éviter de tourner avec les actionneurs déployés.


## Répartition des tâches
Le robot contient deux centres de calculs principaux : le microcontroleur et le microprocesseur.  
> **Le processeur** gère la stratégie haut niveau, la connectivité réseau et l'aquisition et traitement lidar.

> **Le controlleur** gère le controle moteur, l'estimation de position à partir de [encodeuse, centrale inertielle, position absolue résultant du traitement lidar].

Pourquoi? Le controlleur doit gérer le controle moteur le plus précisement possible, et à donc besoin de faire ses calculs en temps réel le plus possible. Le processeur est utile surtout pour la connectivité et sa puissance de calcule au détriment du temps réel. Pour cela ce dernier est réserver à la strat et traitement lidar qui sont respectivement peu critique et lourd en calculs.  
Eventuellement le controlleur peut avoir des élements de détection rapprochés type tof pour une sécurité supplémentaire, et le processeur un algorithme de path planning pour générer des trajectoires courbes.

```Mermaid
C4Component
    title Diagramme C4 - Architecture Microcontrôleur & Raspberry Pi

    System_Boundary(main_robot, "Robot Principal", "Gros robot", "rien") {

        Component_Ext(mag_sens, "As5047p", "Capteur", "Capteur magnétique angle absolue")
        Component_Ext(imu_sens, "ICM42688", "Capteur", "Acceleromètre et gyroscope")
        Component_Ext(lidar, "RPLidar C1", "Capteur", "LIDAR")

        Container_Boundary(mcu, "Microcontrôleur STM32 (Micro-ROS)") {
            Component(t_as5047p, "Encoder-mode Timers", "Hardware", "Aqueri les signaux AB des encodeurs")
            Component(t_imu, "IMU DMA", "Hardware / RTOS Task", "Aqueri les données inertielles")
            Component(t_mot, "Tâche Moteur", "RTOS Timer", "Calcule la vitesse des moteurs (~100Hz)")
            Component(t_sens, "Kalman Filter", "Function", "Lit les données des capteurs physiques")
        }
        Container_Boundary(rpi, "Raspberry Pi (ROS2) ") {
            Component(t_lidar, "sllidar", "Ros2 Node", "Interface le RPLidar-C1 avec Ros2")
            Component(t_obj_det, "Object Detection", "Ros2 Node", "Extrait des formes depuis les données LIDAR")
            Component(t_trig, "Triangulation Balises", "Custom Ros2 Node", "Extrait une position absolue des objets détectés par le LIDAR")
            Component(t_adv_det, "Triangulation Adversaire", "Custom Ros2 Node", "Extrait une position absolue de l'adversaire")
        }
        Container_Boundary(mot, "Bloc Moteur") {
        Component_Ext(esc, "Bg431ESC", "Controlleur de viteses", "Contrôle les moteurs FOC (~20kHz)")
        Component_Ext(mag_sens_esc, "As5047p", "Capteur magnétique", "Angle absolue des roues")
        }
    }    


    %% Communications internes RPi
    Rel(t_lidar, t_obj_det, "Scan")
    Rel_L(t_obj_det, t_trig, "obj")
    Rel_R(t_obj_det, t_adv_det, "obj")
    
    %% Communication internes MCU
    Rel_D(t_sens, t_mot, "pose")
    Rel_D(t_imu, t_sens, "pose")
    Rel_D(t_as5047p, t_sens, "pose")

    %% Communications Rasp - MCU
    Rel_L(t_trig, t_sens, "lidar pose")


    %% Communication internes ESC
    Rel(mag_sens_esc, esc, "SPI : angle absolu")

    %% Communication Robot - MCU
    Rel(t_mot, esc, "CAN : vitesse des roues")
    Rel(mag_sens, t_as5047p, "Signaux AB")
    Rel(imu_sens, t_imu, "SPI")

    %% Communication Robot - Rasp
    Rel(lidar, t_lidar, "câble USB")

```

``` Mermaid
graph TD
    %% Node Styles
    classDef selector fill:#f9f,stroke:#333,stroke-width:2px;
    classDef sequence fill:#bbf,stroke:#333,stroke-width:2px;
    classDef condition fill:#ff9,stroke:#333,stroke-width:1px;
    classDef action fill:#9f9,stroke:#333,stroke-width:1px;

    %% --- LEGEND ---
    subgraph Legend ["Tree Legend"]
        direction LR
        L1{?}:::selector === L1_T["Selector (Runs until a child SUCCEEDS)"]
        L2[➔]:::sequence === L2_T["Sequence (Runs until a child FAILS)"]
        L3([Condition]):::condition === L3_T["Check / State Test"]
        L4[[Action]]:::action === L4_T["Execution / Behavior"]
    end

    %% --- BEHAVIOR TREE ---
    Root((Root)) --> Selector1{?}:::selector
    
    %% Fallback 1: Attack Branch
    Selector1 --> Sequence1[➔ Sequence: Fight]:::sequence
    Sequence1 --> Cond1([Is Enemy Close?]):::condition
    Sequence1 --> Act1[[Attack Enemy]]:::action
    
    %% Fallback 2: Patrol Branch
    Selector1 --> Sequence2[➔ Sequence: Patrol]:::sequence
    Sequence2 --> Cond2([Is Energy High?]):::condition
    Sequence2 --> Act2[[Move to Waypoint]]:::action
    
    %% Fallback 3: Rest Branch
    Selector1 --> Act3[[Sleep/Recover]]:::action
```
