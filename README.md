# Sound Guard - Manual de Uso

## Visão Geral

O Sound Guard é um sistema de monitoramento de níveis sonoros que utiliza um microcontrolador para detectar quando o volume ambiente ultrapassa um limiar pré-definido, acionando um LED de alerta.

## Requisitos do Sistema

### Hardware
- Raspberry Pi 3 ou superior
- Raspberry Pi OS
- Componentes eletrônicos conforme especificação do projeto

### Software para Compilação
- Docker (para cross-compilação)
- Sistema operacional Linux, macOS ou Windows com WSL2

## Compilação

### 1. Preparação do Ambiente

Clone o repositório e navegue até o diretório do projeto:

```bash
git clone https://github.com/Allan-House/Sound_Guard.git
cd Sound_Guard
```

### 2. Build do Container Docker

Construa a imagem Docker para cross-compilação:

```bash
docker build -t sound-guard-builder .devcontainer/
```

### 3. Compilação do Projeto

Execute o container e compile o projeto:

```bash
# Inicie o container
docker run -it --rm -v $(pwd):/workspace sound-guard-builder

# Dentro do container, crie o diretório de build
mkdir -p build
cd build

# Configure o projeto com CMake
cmake ..

# Compile o projeto
make
```

O executável `Sound_Guard` será gerado no diretório `build/bin/`.

### 4. Transferência para Raspberry Pi

Transfira o executável para a Raspberry Pi usando SCP:

```bash
# Substitua <usuario> e <ip-da-raspberry> pelos valores corretos
scp build/bin/Sound_Guard <usuario>@<ip-da-raspberry>:~/
```

**Alternativas de transferência:**
- USB/SD card
- FTP/SFTP
- Compartilhamento de rede

## Operação

### Execução Básica

No Raspberry Pi, execute o programa:

```bash
./Sound_Guard
```

O sistema utilizará o limite padrão de **-12.0 dBFS**.

### Execução com Limite Personalizado

Para definir um limite específico:

```bash
./Sound_Guard -16     # Define limite para -16.0 dBFS
./Sound_Guard -20.5   # Define limite para -20.5 dBFS
```

### Exemplos de Uso

```bash
# Limite mais sensível (detecta sons mais baixos)
./Sound_Guard -25

# Limite menos sensível (detecta apenas sons mais altos)
./Sound_Guard -8

# Usando o padrão
./Sound_Guard
```

## Interface do Sistema

### Display LCD
O sistema exibe continuamente:
- **Linha 1:** "Nivel Medio:"
- **Linha 2:** Valor médio em dBFS (ex: "-15.2 dBFS")

### Terminal
Durante a execução, o terminal mostra:
- Barra visual de volume (80 caracteres)
- Valor RMS em volts
- Valor instantâneo em dBFS
- Média calculada a cada segundo
- Status do LED (on/off)

Exemplo de saída:
```
Volume: ████████████████████                                                     | RMS: 0.125 V | dBFS: -18.1 dB
Average dBFS: -17.3 dB (30 samples in 1.00 s)
LED off..
```

### LED de Alerta
- **LED Ligado:** Nível médio ultrapassou o limite definido
- **LED Desligado:** Nível médio abaixo do limite

## Interpretação dos Valores

### dBFS (Decibels Full Scale)
- **Valores negativos:** Indicam níveis abaixo do máximo
- **Valores próximos de 0:** Indicam níveis altos
- **Exemplo:** -12 dBFS é mais alto que -20 dBFS

### Funcionamento do Limite
- O sistema calcula a média dos níveis a cada segundo
- Se a média ultrapassar o limite, o LED é acionado
- O LED permanece no estado até a próxima medição

## Procedimentos de Operação

### Inicialização
1. Conecte todos os componentes conforme esquema elétrico
2. Ligue a Raspberry Pi
3. Execute o programa
4. Aguarde a mensagem "Iniciando leitura..."

### Durante a Operação
- O sistema opera continuamente
- Não há necessidade de intervenção manual
- O LCD e terminal mostram informações em tempo real

### Encerramento
Para parar o sistema:
```bash
Ctrl + C
```

O programa encerrará de forma segura, exibindo:
- "Encerrando..." no LCD
- "Terminando o programa." no terminal

## Solução de Problemas

### Erro de Inicialização
```
Erro ao inicializar WiringPi.
```
**Solução:** Verifique se o programa está sendo executado como root ou com permissões adequadas.

### Erro de Comunicação I2C
```
Erro ao abrir comunicação I2C com o ADS1115.
```
**Solução:** 
- Verifique as conexões físicas
- Confirme se o I2C está habilitado no Raspberry Pi
- Verifique se os endereços I2C estão corretos

### Erro de LCD
```
Erro ao inicializar LCD I2C.
```
**Solução:**
- Verifique a conexão do LCD
- Confirme o endereço I2C do display (padrão: 0x27)

## Parâmetros de Configuração

### Limite dBFS
- **Padrão:** -12.0 dBFS
- **Faixa sugerida:** -30.0 a -5.0 dBFS
- **Nota:** Valores menos negativos = mais sensível

### Endereços I2C (definidos no código)
- **ADS1115:** 0x48
- **LCD:** 0x27

### GPIO
- **LED:** GPIO 17

## Informações Técnicas

### Taxa de Amostragem
- **Frequência:** ~30 FPS (33.33ms por ciclo)
- **Amostras por medição:** 4
- **Cálculo de média:** A cada 1 segundo

### Precisão
- **Resolução ADC:** 16 bits
- **Faixa de tensão:** ±2.048V
- **Offset DC:** 1.25V (MAX9814)

---

**Nota:** Este manual faz parte da documentação completa do projeto Sound Guard. Para informações sobre arquitetura, esquemas elétricos e especificações técnicas, consulte os demais documentos do projeto.