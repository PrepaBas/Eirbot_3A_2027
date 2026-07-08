FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Paris
ENV ROS_DISTRO=humble
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Basic tools
RUN apt update && apt install -y \
    curl \
    gnupg2 \
    lsb-release \
    build-essential \
    cmake \
    git \
    wget \
    locales \
    sudo \
    software-properties-common \
    && add-apt-repository -y universe \
    && locale-gen en_US.UTF-8 \
    && rm -rf /var/lib/apt/lists/*

# Set the locale
RUN apt-get update && apt-get install -y locales && \
    locale-gen en_US en_US.UTF-8 && \
    update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8 && \
    export LANG=en_US.UTF-8

# ROS 2 repository
RUN curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
    -o /usr/share/keyrings/ros-archive-keyring.gpg

RUN echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] \
    http://packages.ros.org/ros2/ubuntu $(lsb_release -cs) main" \
    > /etc/apt/sources.list.d/ros2.list

# Install ROS 2 Humble (desktop or base)
RUN apt update && apt install -y \
    libserial-dev \
    ros-humble-desktop \
    python3-colcon-common-extensions \
    python3-serial \
    python3-rosdep \
    python3-argcomplete \
    && rm -rf /var/lib/apt/lists/*

RUN apt update && apt install -y \
    ros-humble-twist-mux \
    ros-humble-xacro \
    ros-humble-joint-state-publisher \
    ros-humble-joint-state-publisher-gui \
    ros-humble-robot-localization \
    && rm -rf /var/lib/apt/lists/*

# install Just
RUN curl --proto '=https' --tlsv1.2 -sSf https://just.systems/install.sh | bash -s -- --to /usr/local/bin

# bootstrap rosdep
RUN rosdep init

# Create dev user (recommended)
ARG USERNAME=ros
ARG UID=1000
ARG GID=1000

RUN groupadd -g $GID $USERNAME && \
    useradd -m -u $UID -g $GID -s /bin/bash $USERNAME && \
    echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER $USERNAME
RUN rosdep update --rosdistro $ROS_DISTRO
# 5. Préparation de l'espace de travail
WORKDIR /home/ros/ros2_ws

# On copie le code source
COPY --chown=ros:ros src ./src


RUN echo 'source /opt/ros/humble/setup.bash' >> ~/.bashrc

CMD ["bash"]