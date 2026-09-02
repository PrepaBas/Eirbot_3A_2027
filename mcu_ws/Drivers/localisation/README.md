# Localisation
## Kalman Filter
### Hardware
`kalman_filter.c/h` is a very simple implmentation of an Extended Kalman Filter (EKF) adapted to Eurobot for a differential drive robot using 3 sensors :
- Tables encoder wheels to track the robot's movement on the table isolated from the driving wheels. These are build using As5047p magnetic encoder wheels suspended on linear rails.
- A gyroscope to track the rotation rate. The icm42688 module also gives access to an accelerometer but it is unsed for odometry
- An rpLIDAR C1. This LIDAR works in conjunction with three or more beacons (reflective surfaces) placed on the sides of the table to triangulate the robot's position.  

Even if table wheels encoder and LIDAR could give the robot's odometry, each sensor track one aspect of the movement better than the others. The table wheels excels at mesuring linear movement, the gyroscope gives precise data about rotation rate and the LIDAR gives an absolute position of the robot eliminating any potential drift.

Most importantly, the accelerometer mounted with the gyroscope cannot be used to track linear movement as the error accumulates too fast over time. Additionaly, the LIDAR is too slow (data aquisition and processsing) to give real time odometry data. That is why combining sensors is something to want.

### Aquisition
#### Lecture encodeueses `As5047p`
Ce capteur magnétique peut lire l'angle d'un aimant placé devant lui. La lecture de l'angle peut se faire en SPI mais ici on préfère décoder les signaux en quadrature dit 'AB' que l'as5047p peut générer.  
Le but est d'utiliser les timer STM32 en mode encoder : deux channels d'un timer peuvent faire l'aquisition des signaux AB générés par l'`as5047p`. Dans cubeMX il faut mettre les timer 2 et 5 car ce sont les plus performants et sont sur *32 bits*. Le `Combined Channels` doit être `Encoder Mode`, dans les paramètres en bas le `Encoder Mode` doit être sur TI1 et TI2 pour que la quadrature se fasse. Sur un compteur 32bits, le compteur peut aller jusqu'à 4,294,967,295 ce qui permet de faire (/4000) 1 million de tour de roue sans overflow.  
L'underflow est par contre possible mais tout à fait utile en castant la valeur du compteur en signé `int32_t` dans le programme (alors que le compteur est uint32), ce qui permet d'avoir le compteur **centré en 0**.  
Ainsi l'aquisition des encodeuses se fait sans interuption supplémentaire et ne prend donc pas de cycle de calcul :)  

[outdated] Avec la cinématique inverse, on retrouve la vitesse des roues encodeuses $\dot \omega_r$ et $\dot \omega_l$, et donc la vitesses $\dot x$ et $\dot y$ de la base, mais aussi sa vitesse angulaire $\dot \theta$  qui serviron à estimer la position du robot, dans un filtre Kalman par exemple.

L'initialisation se fait avec la fonctin `
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);`. La lecture des tick mesurés par les timer se font avec la fonction via l'objet timer `TIM2->CNT`. Il ne manque plus qu'à faire la difference avec la précedente mesure et retrouver la distance parcourue par chaque roue encodeuse.

#### Lecture centrale inertielle `ICM42688`
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
> Pourquoi il faut faire ça? En fait pour utliser ces deux capteurs en correction, il faudrait avoir un vecteur d'êtat de taille (5x5) ce qui alourdit enormement les calculs. Ca permet aussi de ne pas avoir à calculer une prédiction mathématique, qui dans tous les cas n'est pas vraiment possible par manque d'un modèle fiable. Donc dans notre montage on est plutot sur une odométrie avec encodeueses et gyroscope normale mais avec une correction LIDAR pondérée.  
C'est pas cool mathématiquement mais on fait de la robotique donc OSEF. Les calculs sont quasiment les même donc je les écris pas. juste $U = \begin{pmatrix} d & \omega \\\end{pmatrix}^T $.