# Imagem base
FROM ubuntu:22.04

# Atualiza pacotes e instala ferramentas necessárias
RUN apt update && apt install -y \
    build-essential \
    gcc \
    g++ \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
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

# Define o compilador padrão para ARM
ENV CC=gcc-aarch64-linux-gnu
ENV CXX=g++-aarch64-linux-gnu

# Adiciona configurações para I2C
RUN echo "i2c-dev" >> /etc/modules && \
    usermod -a -G i2c root

# Define o diretório de trabalho dentro do container
WORKDIR /workspace