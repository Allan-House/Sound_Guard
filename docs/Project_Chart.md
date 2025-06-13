# Sound Guard - Project Chart

## 1. Problemática

A necessidade de monitoramento contínuo de níveis sonoros surge em diversos contextos críticos:

### 1.1 Saúde Auditiva e Bem-estar
- Prevenção de danos auditivos causados por exposição prolongada a ruídos
- Manutenção de ambientes saudáveis em locais de trabalho e estudo
- Cumprimento de normas de segurança ocupacional (NR-15)

### 1.2 Ambientes Críticos
- **Bibliotecas:** Manutenção de ambiente silencioso para concentração
- **Hospitais:** Redução de ruído para recuperação de pacientes e conforto
- **Salas de aula:** Controle acústico para melhor aprendizado
- **Escritórios:** Manutenção de produtividade e conforto dos colaboradores

### 1.3 Monitoramento Industrial
- **Detecção de falhas:** Equipamentos com defeito frequentemente geram ruído anômalo
- **Manutenção preditiva:** Alterações no padrão sonoro podem indicar necessidade de manutenção
- **Controle de processo:** Alguns processos industriais requerem monitoramento acústico

### 1.4 Limitações das Soluções Existentes
- Sistemas comerciais de monitoramento acústico são caros e complexos
- Falta de soluções customizáveis para aplicações específicas
- Dificuldade de integração com sistemas existentes

## 2. Objetivo

Desenvolver um sistema embarcado de baixo custo para monitoramento de níveis sonoros em tempo real, capaz de:
- Detectar automaticamente quando o volume ambiente ultrapassa um limiar configurável
- Fornecer alertas visuais imediatos através de LED
- Exibir informações em tempo real via display LCD e terminal
- Permitir configuração flexível de thresholds via interface CLI
- Operar de forma autônoma com terminação limpa e tratamento de erros
- Servir como base para desenvolvimento de soluções comerciais

## 3. Requisitos Funcionais

### 3.1 Aquisição de Dados
- Capturar sinais de áudio ambiente continuamente
- Processar sinais em tempo real com taxa de ~30 FPS
- Calcular valores RMS e converter para escala dBFS
- Realizar média móvel a cada segundo (30 amostras)

### 3.2 Processamento
- Aplicar threshold configurável (padrão: -12.0 dBFS)
- Comparar média calculada com threshold definido
- Suportar faixa de threshold de -30.0 a -5.0 dBFS
- Processar argumentos de linha de comando para configuração

### 3.3 Interface e Alertas
- Acionar LED quando threshold for ultrapassado
- Exibir informações no LCD (nível médio em dBFS)
- Mostrar barra visual de volume no terminal
- Apresentar valores instantâneos e médias calculadas

### 3.4 Controle e Interface de Usuário
- Inicialização automática do sistema
- Suporte a argumentos de linha de comando:
  - `-l` ou `--limit VALOR`: Define threshold personalizado
  - `-h` ou `--help`: Exibe ajuda detalhada
- Validação de parâmetros com avisos para valores positivos
- Encerramento seguro via Ctrl+C (SIGINT handler)
- Tratamento de erros de comunicação I2C
- Feedback visual e textual em tempo real

## 4. Restrições

### 4.1 Hardware
- Deve operar em Raspberry Pi 3 ou superior
- Utilizar apenas componentes de baixo custo
- Comunicação limitada a protocolos I2C e GPIO
- Alimentação via fonte da Raspberry Pi

### 4.2 Software
- Compatível com Raspberry Pi OS
- Programação em C/C++ usando WiringPi
- Cross-compilação via Docker
- Sem dependências de bibliotecas externas complexas

### 4.3 Operacionais
- Operação contínua sem supervisão
- Precisão limitada pela resolução do ADC (16 bits)
- Taxa de amostragem fixa (~30 FPS)
- Sem armazenamento de histórico de dados
- **Protótipo:** Versão atual é prova de conceito, implementação comercial exigiria PCB dedicada e componentes industriais

## 5. Descrição Sucinta da Solução

O Sound Guard é um sistema embarcado baseado em Raspberry Pi que monitora continuamente os níveis sonoros ambiente através de um microfone amplificado (MAX9814). O sinal analógico é digitalizado por um conversor ADC de 16 bits (ADS1115) via comunicação I2C. O software processa os dados em tempo real, calcula valores RMS e os converte para escala dBFS, mantendo uma média móvel atualizada a cada segundo. Quando a média ultrapassa o threshold configurado, um LED de alerta é acionado via GPIO. As informações são exibidas simultaneamente em um display LCD I2C e no terminal, fornecendo feedback visual contínuo do estado do sistema.

## 6. Especificação

### 6.1 Arquitetura do Sistema
- **Microcontrolador:** Raspberry Pi 3+ com ARM Cortex-A53
- **OS:** Raspberry Pi OS (Linux)
- **Linguagem:** C/C++ com WiringPi library
- **Comunicação:** I2C para ADC e LCD, GPIO para LED

### 6.2 Parâmetros de Operação
- **Taxa de amostragem:** ~30 FPS (33.33ms por ciclo)
- **Resolução ADC:** 16 bits
- **Faixa de tensão:** ±2.048V
- **Offset DC:** 1.25V (pré-amplificador)
- **Threshold padrão:** -12.0 dBFS
- **Média móvel:** 30 amostras/segundo

### 6.3 Endereçamento I2C
- **ADS1115 (ADC):** 0x48
- **LCD 16x2:** 0x27
- **LED de alerta:** GPIO 17

### 6.4 Interface de Usuário
- **Display LCD:** 
  - Linha 1: "Nivel Medio:"
  - Linha 2: Valor médio formatado (ex: "-15.2 dBFS")
  - Atualização a cada cálculo de média (1 segundo)
- **Terminal:** 
  - Barra visual de volume em tempo real
  - Valores instantâneos (RMS em volts, dBFS instantâneo)
  - Relatório de média com estatísticas (amostras, tempo decorrido)
  - Status do LED (on/off) após cada decisão
- **Configuração:** 
  - Parâmetros: `-l/--limit VALOR` para threshold
  - Help integrado: `-h/--help`
  - Validação com confirmação para valores não-convencionais

### 6.5 Algoritmo de Processamento
```
1. Leitura ADC → Cálculo RMS → Conversão dBFS → Display instantâneo
2. Acumulação de amostras durante 1 segundo
3. Cálculo de média aritmética
4. Comparação com threshold → Decisão LED
5. Atualização LCD e reset do acumulador
6. Repetição do ciclo
```

### 6.6 Arquitetura de Software
- **Modularização:**
  - `main.c`: Controle principal e interface CLI
  - `config.h`: Definições e constantes
  - `lcd.h/c`: Controle do display LCD
  - `adc.h/c`: Interface com ADS1115
  - `audio.h/c`: Processamento de áudio e cálculos
  - `timing.h/c`: Controle de temporização
- **Padrões de Design:**
  - Signal handling para terminação limpa
  - Separação de responsabilidades por módulos
  - Tratamento robusto de erros
  - Interface CLI amigável com validação

## 7. Lista dos Componentes

### 7.1 Componentes Principais
| Componente | Modelo/Tipo | Quantidade | Função |
|------------|-------------|------------|---------|
| Microcontrolador | Raspberry Pi 3 | 1 | Processamento principal |
| Conversor ADC | ADS1115 | 1 | Digitalização do sinal de áudio |
| Amplificador de Microfone | MAX9814 | 1 | Pré-amplificação e condicionamento |
| Display LCD | GDM1602K | 1 | Display LCD 16x2 |
| Adaptador I2C | Módulo Serial I2C | 1 | Interface I2C para LCD |
| LED | LED 5mm (vermelho) | 1 | Alerta visual |
| Resistor | 220Ω 1/4W | 1 | Limitação de corrente do LED |

### 7.2 Componentes de Suporte
| Componente | Especificação | Quantidade | Função |
|------------|---------------|------------|---------|
| Cabos Jumper | Macho-Fêmea | 10 | Conexões |
| Protoboard | 830 pontos | 1 | Montagem do circuito |
| Cartão MicroSD | 32GB Classe 10 | 1 | Armazenamento do OS |

### 7.3 Alimentação
- **Fonte:** Alimentação via Raspberry Pi (5V via USB)
- **Consumo estimado:** 2-3W total do sistema

### 7.4 Software/Ferramentas
| Item | Versão/Tipo | Função |
|------|-------------|---------|
| Raspberry Pi OS | Latest | Sistema operacional |
| WiringPi | Latest | Biblioteca GPIO/I2C |
| CMake | 3.16+ | Sistema de build |
| Docker | Latest | Cross-compilação |
| GCC Cross-compiler | ARM | Compilação cruzada |

### 7.5 Considerações para Versão Comercial
- **PCB dedicada** para maior confiabilidade e compactação
- **Componentes SMD** para redução de tamanho
- **Microcontrolador dedicado** (ex: STM32) para menor consumo
- **Case/encapsulamento** para proteção industrial
- **Certificações** (CE, FCC) para uso comercial

### 7.6 Estimativa de Custo (Protótipo)
- **Raspberry Pi 3:** ~R$ 200
- **Módulos (ADS1115, MAX9814):** ~R$ 60
- **Display e adaptador I2C:** ~R$ 30
- **Componentes diversos:** ~R$ 20
- **Total estimado:** ~R$ 310

---

**Data de Elaboração:** Junho 2025  
**Versão:** 1.0  
**Status:** Implementado