# Imagem base
FROM ubuntu:22.04

# Atualiza pacotes e instala ferramentas necessárias
RUN apt update && apt install -y \
    build-essential \
    gcc-arm-linux-gnueabihf \
    g++-arm-linux-gnueabihf \
    cmake \
    make \
    git \
    nano \
    vim \
    gdb-multiarch \
    pkg-config \
    libssl-dev \
    curl \
    ca-certificates \
    wiringpi \
    libwiringpi-dev \
    i2c-tools \
    libi2c-dev

# Define o compilador padrão para ARM 32-bit
ENV CC=arm-linux-gnueabihf-gcc
ENV CXX=arm-linux-gnueabihf-g++

# Adiciona configurações para I2C
RUN echo "i2c-dev" >> /etc/modules && \
    usermod -a -G i2c root

# Define o diretório de trabalho dentro do container
WORKDIR /workspace