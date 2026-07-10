# EIRBOT 3A 2027
## Equipe
- Barthere Aurelien E3
- Bromet Alexandre E3
- Daragnes Leïa I3
- Guigui Marceau E3
- Halimi Zakaria E3
- Louvel Elise E3
- Marie Quentin E3
- Tran-Ruesche Bastien E3

#### Traquenard
- Berthod Hugo E3
- Donias Arno I3
- Michaud Lou-Ann E3
- Maxime Lavit E3

#### Refus
- 

checkout to humble then go back to main folder and
``` bash
docker pull microros/micro_ros_static_library_builder:humble
docker run -it --rm -v $(pwd):/project --env MICROROS_LIBRARY_FOLDER=micro_ros_stm32cubemx_utils/microros_static_library microros/micro_ros_static_library_builder:humble
```

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
Framwork Ros2 dans un docker pour pouvoir laisser raspos en natif. Le but est de commencer à écrire nos propres nodes. La strat générale c'est d'utiliser les Behavior Tree pour séquencer les actions : il faut donc faire des serveurs
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
Un lidar si on veut branché sur la rasp et qui permet de faire de l'évitement longue distance  
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
            Component(t_imu, "IMU DMA and accumulation", "Hardware / RTOS Task", "Aqueri les données inertielles")
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


### Lecture encodeueses `As5047p`
Ce capteur magnétique peut lire l'angle d'un aimant placé devant lui. La lecture de l'angle peut se faire en SPI mais ici on préfère décoder les signaux en quadrature dit 'AB' que l'as5047p peut générer.  
Le but est d'utiliser les timer STM32 en mode encoder : deux channels d'un timer peuvent faire l'aquisition des signaux AB générés par l'`as5047p`. Dans cubeMX il faut mettre les timer 2 et 5 car ce sont les plus performants et sont sur *32 bits*. Le `Combined Channels` doit être `Encoder Mode`, dans les paramètres en bas le `Encoder Mode` doit être sur TI1 et TI2 pour que la quadrature se fasse. Sur un compteur 32bits, le compteur peut aller jusqu'à 4 294 967 295 ce qui permet de faire (/4000) 1 million de tour de roue sans overflow.  
L'underflow est par contre possible mais tout à fait utile en castant la valeur du compteur en signé `int32_t` dans le programme (alors que le compteur est uint32), ce qui permet d'avoir le compteur **centré en 0**.  
Ainsi l'aquisition des encodeuses se fait sans interuption supplémentaire et ne prend donc pas de cycle de calcul :)  

[outdated] Avec la cinématique inverse, on retrouve la vitesse des roues encodeuses $\dot \omega_r$ et $\dot \omega_l$, et donc la vitesses $\dot x$ et $\dot y$ de la base, mais aussi sa vitesse angulaire $\dot \theta$  qui serviron à estimer la position du robot, dans un filtre Kalman par exemple.

L'initialisation se fait avec la fonctin `
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);`. La lecture des tick mesurés par les timer se font avec la fonction via l'objet timer `TIM2->CNT`. Il ne manque plus qu'à faire la difference avec la précedente mesure et retrouver la distance parcourue par chaque roue encodeuse.

### Lecture centrale inertielle `ICM42688`
L'ICM42688 est un capteur d'acceleration et de rotation, donc de 6 valeurs (x, y, z pour chaque). Celà veut dire une lecture des registres en commande SPI de manière assez fréquente (>500Hz). Pour celà, la centrale génère une interuption via ses pin (le interrupt 1), et une lecture des registres peut ainsi suivre. Pour encore moins d'utilisation des cycles, il est possible d'utiliser la lecture en Direct Memory Access (DMA) ce qui donne l'execution suivante : interuption levée -> Envoi de la commande et lecture des registres en DMA en background -> interruption du DMA pour signaler la fin de l'aquisition -> calcul léger pour intégrer les valeurs.  
De manière moins fréquente, l'accumulation des valeurs peut être utilisée pour estimer l'état du robot dans un filtre Kalman par exemple.  

Il faut donc :
- **Un SPI** - ici le 2, le 1 étant restrint par le timer2 dand cubeMX. Préscaler de 16 pour rester en dessous de 24MHz, mots de 8 bits. Channel DMA en PAS circular car on sait à l'avance combient d'octets on veut lire. Bien selectionner le `Request` et configurer le `Direction` en conséquence. Pour les deux channel faire en sorte que l'incrément d'adresse se fasse sur la ram et non pas sur l'IMU.
- **Le pin CS** pour compléter le SPI (le mettre rapide).
- **Un pin d'intéruption** qui est activé par l'IMU. Cette intéruption doit trigger un sémaphore/notification pour mettre CS bas et déclencher la transmission DMA dans une tâche.

Une fonction d'envoi SPI consist à : 1.préparer le packet, 2.baisser le CS, 3.trasmettre avec `HAL_SPI_Transmit`, 4.remonter le CS.  
On utilise cette fonction pour configurer l'IMU.  

Les intéruptions utilisent le système de notificatin (similaire au sémaphore) pour débloquer la tâche principale. Il faut juste donner la *handle* de la tâche (récupérée avec`xTaskGetCurrentTaskHandle()`) et utiliser les fonctions `ulTaskNotifyTake(pdTRUE, portMAX_DELAY)` et `TaskNotifyGiveFromISR(imu.notified_task_handle, &xHigherPriorityTaskWoken)`. A noter qu'on ajoute la fonction `portYIELD_FROM_ISR(xHigherPriorityTaskWoken)` dans l'intéruption pour être sur de changer de tâche rapidement.

Le résultat des mesures peut être mis dans des variables globales ect mais de manière plus sécurisé on utilise les queue. On créé une queue et on récupère sa *handle* `imu_queue_handle = xQueueCreate(3, sizeof(imu_data_t))`, et à la fin du traitement on poste les données `xQueueSend(imu_queue_handle, &package, 0)`. Dans une autre tâche (kalman par exemple) on attend puis récupère les données `if (xQueueReceive(imu_queue_handle, &imu_packet, portMAX_DELAY) == pdPASS) {..}.`

Ensuite il faut parser les données pour pouvoir les utiliser.

### Le filtre de Kalman
L'odométrie du robot peut se faire de plusieurs façons :
- j'ai demandé telle et telle vitesse à mes roues, je peux donc savoir ou je vais être à l'instant suivant
- Mes capteurs ont mesuré un déplacement de 3cm, je sais donc ou je me trouve à partir de ma position précédente
- Mon lidar à permis de me trianguler en x- y-, c'est donc ici que je me trouve.

Celà permet de faire la distinction entre trois type de donnée : **La prédiction** (le calcul à partir des info déjà aquises); **l'estimation relative** (à partir d'une donné précedente) et **l'estimation absolue** (indépendant de l'état précedent). 
La prédiction est bien car elle permet à tout moment d'estimer l'état futur mais est sujette à la dérive car une commande ne se traduit jamais parfaitement dans la réalité. 
L'estimation relative (dead reckoning) permet d'avoir une donnée supplémentaire en ajoutant un capteur qui va mesurer une donnée réelle. Cependant, même si il peut mieux traduire le comportement réel du robot il est toujours victime d'une dérive avec le temps. 
L'estimation absolue en se fie pas du tout à l'état passé et trouve une position absolue pour le robot en se basant par exemple sur un lidar, gps, ect. Cette estimation ne dérive pas mais est souvent bruitée ou en retard ne la rendant pas viable pour un système rapide ou très précis.

Pour utiliser les forces et faiblesses de toutes ces estimations, on va essayer de faire de la **fusion de capteur**. C'est à dire se fier tout le temps aux calculs mathématiques si notre modèle n'est pas trop mauvais, écouter les capteurs relatifs car ils reflètent mieux la réalité et enfin écouter les capteurs absolue pour éliminer la dérive. C'est l'objectif d'un filtre de Kalman.  
Cependant le filtre ne fait pas une simple moyenne des capteurs. Il joue avec les incertitudes pour savoir si une donnée doit être prise en compte ou pas. Dans les faits on définit des matrices de bruit pour les maths (traduit un peu la réalité eg une table mal faite) et les capteurs. A chaque calcul mathématique, l'erreur est propagée (logique on est de moins en moins sur de où on est). A chaque donnée, le filtre Kalman va mathématique donner du poid ou non à la mesure suivant son bruit et le l'incertitude atteinte par le systeme, et l'intégrer à la position du robot. 
Celà fait en sorte qu'une mesure bruitée ne pertube pas une position certaine et à l'inverse qu'une position incertaine soit complémentée par une mesure (même bruitée).

>**Les équations** sont les suivantes ($f$ est une fonction non linéaire donc on utilise un filtre de kalman étendu)
>- **Prédiction** (à Priori)
    - Prédiction d'êtat $$\hat x_{k|k-1} = f(\hat x_{k-1|k-1}, u_{k-1})$$
    - Prédiction de covariance $$P_{k|k-1} = F_k P_{k-1|k-1}F^T_k + Q_{k-1}$$
>- **Correction** (à Postériori)
    - Innovation $$\tilde y_k = z_k - h(\hat x_{k|k-1})$$
    - Covariance de  l'innovatin  $$S_k = H_k P_{k|k-1} H^T_k + R_k$$
    - Gain de Kalman quasi-optimal $$K_k = P_{k|k-1} H^T_k S^{-1}_k$$
    - Etat mis à jour $$\hat x_{k|k} = \hat x_{k|k-1} + K_k \tilde y_k$$
    - Covariance mise à jour $$P_{k|k} = (I - K_kH_k)P_{k|k-1}$$
>avec 
    - Matrice d'évolution $$F_k = \frac{\partial f}{ \partial x}|_{\hat x_{k-1|k-1},  u_{k-1}}$$
    - Matrice d'observation $$H_k = \frac{\partial h}{ \partial x}|_{\hat x_{k|k-1}}$$



On distingue plusieurs variantes :
- $\hat x_{k|k}$ l'estimation de x que l'on cherche à calculer (prédiction plus correction)
- $ \hat x_{k|k-1}$ l'estimation (aucune nouvelle donnée seuelement les commandes)
- $\hat x_{k-1|k-1}$ ancienne donnée ($\hat x_{k|k}$ du cycle d'avant)
a confiance en la position
- $z_k$ est la mesure
- $F$ est la jacobienne de l'équation de propagation. Equation de propagation qui prend les commandes utilisateur et calcul l'état suivant théorique  
- $P$ porte l'incertitude de la position $x$ sous forme de covariances  
- $Q$ porte l'incertitude qui doit être rajouté par la simple existance du système (glissement ..)  
- $H$ sert à transposer la donnée capteur (ou la différence si on parle de l'innovation) sous le format du vecteur d'êtat (e.g., vitesse(capteur) * $\Delta t$ )  
- $R$ traduit le bruit de la mesure. Propre à chaque capteur  
- $S$ est juste la matrice à inverser pour le gain de Kalman. (elle met le bruit de mesure au dénominateur)
 
**Remarque** : Il n'y à aucun bruit dans les équations parce que cette donnée est contenue dans les matrices $Q$ et $R$. Une modélisation de la réalité est $x_k = Fx_{k-1} + Bu_k+ bruit$ mais le but du filtre est d'abstraire ce bruit et de le filtrer de manière probabilistique.

>**Prédiction robot différentiel**
>- **Données** : $v_l$ et $v_r$ la commande de vitesse (à $t=k-1$) au sol des roues, $L$ l'entraxe (roues motrices)
>- **Vecteurs êtat** et **commande** : $$\hat X = \begin{bmatrix}x \\y \\\theta\end{bmatrix} , U= \begin{bmatrix}v_l \\ v_r \\\end{bmatrix}$$
>- **Equations de propagation** $f(\hat x_{k-1|k-1}, u_{k-1})$ : $$x_k = x_{k-1} + \frac{v_l + v_r}{2} cos(\theta_{k-1})\Delta t$$ $$y_k = y_{k-1} + \frac{v_l + v_r}{2} sin(\theta_{k-1})\Delta t$$ $$\theta_k = \theta_{k-1} - \frac{v_l - v_r}{L} \Delta t$$
> =>$$F= \begin{bmatrix} 1 & 0 & -\frac{v_l + v_r}{2} sin(\theta_{k-1})\Delta t \\ 0 & 1 & \frac{v_l + v_r}{2} cos(\theta_{k-1})\Delta t \\ 0 & 0 & 1 \end{bmatrix}$$
>- **$P$** (3x3) est recalculée en permanance. Sa valeur initiale peut être 0 si on est sûr de la position du robot (repositionnement).
>- **$Q$** (3x3) est normalement dynamique en fonction de la vitesse du robot qui fait augmenter l'incertitude de la prédiction. Cependant une matrice constante déterminée empiriquement est beaucoup plus simple à mettre en place. Des valeurs élevées vont faire que les capteurs seront beaucoup plus pris en compte et inversement.

---

>**Correction avec LIDAR**  
>Techniquement on peut garder le formalisme matriciel et avoir $H$ de taille (6x3) mais en pratique ce n'est pas pratique car les données n'arrivent pas en même temps et les calculs sont très lourds. On préfère faire correction par correction (ce qui revient au même car c'est des multiplications) pour gagner en flexibilité.
>- **LIDAR** triangulé donne une position de la même forme que le vecteur d'état.  
    - $H = \begin{pmatrix} 1&0&0\\0&1&0\\0&0&1\\\end{pmatrix}$ et $R$ à définir suivant la précision du positionnement

---

>**Et mes autres capteurs?????**  
> **/!\\** En fait on va pas du tout faire comme la théorie et plutot utiliser **les roues encodeuses** et **la centrale inertielle** pour faire la prédiction. *Contre-intuitif?? Bah oui...*  
Ducoup les **roues encodeuses** vont nous donner une donnée de distance parcourue dont on gardera que la distance linéaire (demi-somme des distance des deux roues).  
L'**IMU** lui va donner la donnée de rotation sur l'axe z, les autres données (acceleromètre et les deux autres axes du gyroscope) ne sont pas utilisées.   
On utilise les valeurs dans un nouveau vecteur commande.
>
> Pourquoi il faut faire ça? En fait pour utliser ces deux capteurs en correction, il faudrait avoir un vecteur d'êtat de taille (5x5) ce qui alourdit enormement les calculs. Ca permet aussi de ne pas avoir à calculer une prédiction mathématique, qui dans tous les cas n'est pas vraiment représentative du mouvement du robot (grosses accelerations, collisions...). Donc dans notre setup on est plutot sur une odométrie avec encodeueses et gyroscope normale mais avec une correction LIDAR pondérée.  
C'est pas cool mathématiquement mais on fait de la robotique donc OSEF. Les calculs sont quasiment les même donc je les écris pas. juste $U = \begin{pmatrix} d & \omega \\\end{pmatrix}^T $.