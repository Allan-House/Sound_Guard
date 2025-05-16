# Imagem base
FROM ubuntu:22.04

# Atualiza pacotes e instala ferramentas necessárias
RUN apt update && apt install -y \
    build-essential \
    gcc \
    g++ \
    gcc-arm-linux-gnueabihf \
    g++-arm-linux-gnueabihf \
    cmake \
    make \
    git \
    wget \
    ssh \
    nano \
    vim \
    pkg-config \
    #
    gdb-multiarch \
    libssl-dev \
    curl \
    ca-certificates \
    wiringpi \
    libwiringpi-dev \
    i2c-tools \
    libi2c-dev

# Define o compilador padrão para ARM 32-bit
ENV CC=gcc-arm-linux-gnueabihf
ENV CXX=g++-arm-linux-gnueabihf

# Adiciona configurações para I2C
RUN echo "i2c-dev" >> /etc/modules && \
    usermod -a -G i2c root

# Define o diretório de trabalho dentro do container
WORKDIR /workspace