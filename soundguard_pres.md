---
marp: true
theme: default
class: lead
paginate: true
backgroundColor: #1a1a1a
color: #ffffff
style: |
  section {
    background: linear-gradient(135deg, #1a1a1a 0%, #2d2d2d 100%);
  }
  h1, h2 {
    color: #4CAF50;
  }
  .highlight {
    background: #4CAF50;
    color: #000;
    padding: 0.2em 0.5em;
    border-radius: 4px;
  }
  .small-text {
    font-size: 0.8em;
  }
  .tiny-text {
    font-size: 0.6em;
  }
---

# 🔊 Sound Guard
## Sistema de Monitoramento de Níveis Sonoros

**Projeto de Sistema Embarcado com IoT**

---

## 📋 Visão Geral

- **Sistema de monitoramento** de níveis sonoros em tempo real
- Utiliza **Raspberry Pi** para processamento
- **Alerta visual** com LED quando limiar é ultrapassado
- **Interface LCD** para visualização local
- **Desenvolvido em C/C++** com CMake

---

## 🎯 Objetivos

### Principais funcionalidades:
- ✅ Monitoramento contínuo de níveis sonoros
- ✅ Alerta visual quando limite é ultrapassado
- ✅ Display em tempo real no LCD e terminal
- ✅ Configuração personalizável de limites
- ✅ Processamento de média em tempo real

---

## 🏗️ Arquitetura do Sistema

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Microfone     │ -> │  ADS1115 (ADC)  │ -> │  Raspberry Pi   │
│   (MAX9814)     │    │                 │    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                        │
                           ┌────────────────┬───────────┴─────────────┐
                           │                │                         │
                    ┌─────────────┐  ┌─────────────┐       ┌─────────────┐
                    │    LCD      │  │  LED Alert  │       │  Terminal   │
                    │  (I2C 0x27) │  │ (GPIO 17)   │       │   Output    │
                    └─────────────┘  └─────────────┘       └─────────────┘
```

---
<style scoped>
section {
    font-size: 26px;
}
</style>


## 🔧 Componentes Necessários

### Hardware:
- **Raspberry Pi 3** ou superior
- **ADS1115** - Conversor Analógico-Digital (16-bit)
- **MAX9814** - Amplificador de microfone
- **LCD I2C** - Display 16x2 (endereço 0x27)
- **LED** - Indicador visual (GPIO 17)
- Componentes eletrônicos básicos

### Software:
- Raspberry Pi OS
- Docker (para cross-compilação)
- CMake, WiringPi

---
<style scoped>

table {
  background-color: #2d2d2d;
  color: #ffffff;
}
table th {
  background-color:rgb(255, 255, 255);
  color: #000000;
}
table td {
  background-color: #2d2d2d;
  color: #ffffff;
  border: 1px solid #555;
}
</style>

## 📊 Especificações Técnicas

| Parâmetro | Valor |
|-----------|-------|
| **Resolução ADC** | 16 bits |
| **Faixa de tensão** | ±2.048V |
| **Frequência de amostragem** | ~30 FPS |
| **Amostras por medição** | 4 |
| **Cálculo de média** | A cada 1 segundo |
| **Limiar padrão** | -12.0 dBFS |
| **Faixa sugerida** | -30.0 a -5.0 dBFS |

---

## 🚀 Como Funciona

### Fluxo de Operação:
1. **Captura** contínua do áudio via microfone
2. **Conversão** analógico-digital pelo ADS1115  
3. **Processamento** dos dados na Raspberry Pi
4. **Cálculo** da média RMS a cada segundo
5. **Comparação** com limiar configurado
6. **Acionamento** do LED se limiar for ultrapassado
7. **Exibição** dos dados no LCD e terminal

---

## 🎮 Utilização

### Execução básica:
```
./Sound_Guard              # Usa limiar padrão (-12.0 dBFS)
```

### Configuração personalizada:
```
./Sound_Guard -16          # Limiar: -16.0 dBFS
./Sound_Guard -20.5        # Limiar: -20.5 dBFS
./Sound_Guard -25          # Mais sensível (sons baixos)
./Sound_Guard -8           # Menos sensível (sons altos)
```
---

## 📱 Interface do Usuário

### Display LCD (16x2):
Linha 1: "Nivel Medio:"
Linha 2: "-15.2 dBFS"

### Terminal:
```
Volume: ████████████████████ | RMS: 0.125 V | dBFS: -18.1 dB
Average dBFS: -17.3 dB (30 samples in 1.00 s)
LED off..
```
### Indicadores:
- 🟢 **LED Desligado**: Nível abaixo do limite
- 🔴 **LED Ligado**: Nível ultrapassou o limite

---

## 📈 Interpretação dos Dados

### Escala dBFS:
- **Valores negativos**: Níveis abaixo do máximo
- **Próximo de 0**: Níveis altos
- **-12 dBFS** > **-20 dBFS** (mais alto)

### Exemplo prático:
- **-25 dBFS**: Ambiente muito silencioso
- **-12 dBFS**: Conversação normal
- **-5 dBFS**: Ambiente barulhento

---
<style scoped>

table {
  background-color: #2d2d2d;
  color: #ffffff;
}
table th {
  background-color:rgb(255, 255, 255);
  color: #000000;
}
table td {
  background-color: #2d2d2d;
  color: #ffffff;
  border: 1px solid #555;
}
</style>

## 🛠️ Solução de Problemas

### Erros comuns:

| Erro | Solução |
|------|---------|
| WiringPi falhou | Executar como root/sudo |
| ADS1115 não detectado | Verificar conexões I2C |
| LCD não responde | Confirmar endereço 0x27 |
| Permissões GPIO | Adicionar usuário ao grupo gpio |

---

## 🔍 Monitoramento em Tempo Real

### Sistema exibe continuamente:
- **Barra visual** de volume (80 caracteres)
- **Valor RMS** em volts
- **dBFS instantâneo**
- **Média calculada** por segundo
- **Status do LED** (on/off)

### Para parar: Ctrl + C
Encerrando...
Terminando o programa.

---

## 🎯 Aplicações Práticas

### Casos de uso:
- 🏠 **Monitoramento doméstico** de ruído
- 🏭 **Controle industrial** de níveis sonoros
- 🎵 **Estúdios** e salas de ensaio
- 🏫 **Ambientes educacionais**
- 🏥 **Áreas hospitalares**

---

## 📋 Próximos Passos

### Melhorias futuras:
- 📱 Interface web para monitoramento remoto
- 📊 Logging e histórico de dados
- 🔔 Notificações via webhook/email
- 📈 Análise espectral avançada
- 🌐 Integração com IoT platforms

---

# Obrigado

### Repositório:
**GitHub**: [Allan-House/Sound_Guard](https://github.com/Allan-House/Sound_Guard)

Allan Ronaldo Monteiro Barbosa
Henry Gabriel Cavalheiro Fillvock
Júlio César Foltran Cordeiro
Pedro Henrique Sturaro Calegari