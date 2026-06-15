# Robô AGV Seguidor de Linhas

Trabalho da disciplina de Sistemas Embarcados — Robô Auto Guiado (AGV)

## Descrição

Robô autônomo seguidor de linhas capaz de navegar em um grid demarcado no chão, executar um caminho pré-determinado e desviar de obstáculos automaticamente. Implementado em Arduino com máquina de estados finita.

## Objetivo

Desenvolver um AGV (Automated Guided Vehicle) capaz de:
- Trafegar pelas linhas de um grid 4×4
- Executar um caminho definido por uma sequência de decisões em cada intersecção
- Detectar e aguardar a remoção de obstáculos (anti-colisão)
- Operar de forma totalmente autônoma

## Grid

```
     E    F    G    H
1 ---+----+----+----+--- 5
     |    |    |    |
2 ---+----+----+----+--- 6
     |    |    |    |
3 ---+----+----+----+--- 7
     |    |    |    |
4 ---+----+----+----+--- 8
     A    B    C    D
```

## Hardware

| Componente              | Descrição                                   |
|-------------------------|---------------------------------------------|
| Arduino Uno/Nano        | Microcontrolador principal                  |
| 2× Sensor IR seguidor   | Leitura da linha (esquerdo e direito)       |
| 1× Sensor IR contador   | Detecção de intersecções via interrupção    |
| 1× Sensor IR obstáculo  | Anti-colisão via interrupção                |
| 2× Motor DC             | Tração diferencial                          |
| Driver ponte H (L298N)  | Controle de direção e velocidade dos motores|

## Mapeamento de Pinos

| Pino | Função                  | Tipo     |
|------|-------------------------|----------|
| 2    | Sensor obstáculo        | INPUT    |
| 3    | Sensor contador de linha| INPUT    |
| 5    | PWM motor direito       | OUTPUT   |
| 6    | PWM motor esquerdo      | OUTPUT   |
| 7    | Motor esquerdo IN_1     | OUTPUT   |
| 8    | Motor esquerdo IN_0     | OUTPUT   |
| 9    | Motor direito IN_1      | OUTPUT   |
| 10   | Motor direito IN_0      | OUTPUT   |
| 11   | Sensor seguidor direito | INPUT    |
| 12   | Sensor seguidor esquerdo| INPUT    |

## Máquina de Estados

```
SEGUIR
  │
  ├── sensor conta intersecção → decisao = DIREITA  ──► CURVA_D ──► volta pra SEGUIR
  ├── sensor conta intersecção → decisao = ESQUERDA ──► CURVA_E ──► volta pra SEGUIR
  ├── sensor conta intersecção → decisao = FIM      ──► PARADO_FIM (fica pra sempre)
  │
  └── obstáculo detectado ──► PARADO ──► obstáculo removido ──► volta pro estado anterior
```

### Estados

| Estado       | Descrição                                              |
|--------------|--------------------------------------------------------|
| `SEGUIR`     | Segue a linha usando os sensores esquerdo e direito    |
| `CURVA_D`    | Executa curva de 90° para a direita                    |
| `CURVA_E`    | Executa curva de 90° para a esquerda                   |
| `PARADO`     | Para aguardando remoção do obstáculo (retomável)       |
| `PARADO_FIM` | Para permanentemente ao concluir o caminho             |

## Sistema de Sequência de Decisões

Cada intersecção detectada pelo sensor contador (`IR_CONTADOR_LINHA`) consome um elemento da sequência:

```cpp
int sequencia[] = {FRENTE, DIREITA, ESQUERDA, FRENTE, FRENTE};
```

| Valor      | Ação na intersecção              |
|------------|----------------------------------|
| `FRENTE`   | Passa reto, sem virar            |
| `DIREITA`  | Vira 90° à direita               |
| `ESQUERDA` | Vira 90° à esquerda              |
| `FIM`      | Para permanentemente             |

### Como definir o caminho

1. Identifique as intersecções que o robô vai cruzar, **em ordem**
2. Para cada uma, defina a ação: `FRENTE`, `DIREITA`, `ESQUERDA` ou `FIM`
3. Ajuste o array `sequencia[]` e `totalPassos` no código

**Exemplo** — caminho `1 → E → 2 → B → 4`:
```
Intersecção 1 (cruzamento em E): passa reto   → FRENTE
Intersecção 2 (cruzamento em 2): vira direita → DIREITA
Intersecção 3 (cruzamento em B): vira direita → DIREITA
Intersecção 4 (chegou em 4):     para         → FIM
```
```cpp
int sequencia[]   = {FRENTE, DIREITA, DIREITA, FIM};
const int totalPassos = 4;
```

## Anti-Colisão

O sensor de obstáculo (`IR_OBS`, pino 2) é ligado via interrupção `CHANGE`:

- **Obstáculo detectado** (sinal LOW): robô para imediatamente, salva o estado atual
- **Obstáculo removido** (sinal HIGH): robô retoma o estado salvo automaticamente

```cpp
void obsISR()
{
  obstaculoAtivo = (digitalRead(IR_OBSTACULO) == LOW);
}
```

## Parâmetros Ajustáveis

```cpp
#define V_FRENTE_DIREITO   200   // velocidade roda direita em linha reta
#define V_FRENTE_ESQUERDO  128   // velocidade roda esquerda em linha reta

#define V_CURVA_D_RAPIDO   170   // roda rápida na curva direita
#define V_CURVA_D_LENTO     10   // roda lenta na curva direita

#define V_CURVA_E_RAPIDO   240   // roda rápida na curva esquerda
#define V_CURVA_E_LENTO     10   // roda lenta na curva esquerda

#define T_CURVA_D_MIN     1000   // tempo mínimo de curva direita (ms)
#define T_CURVA_E_MIN     1000   // tempo mínimo de curva esquerda (ms)

#define T_CURVA_D_MAX     7500   // timeout curva direita (ms)
#define T_CURVA_E_MAX     7500   // timeout curva esquerda (ms)
```

> Os valores de `V_FRENTE` e `V_CURVA` variam entre robôs. Calibre conforme o hardware.

## Inicialização com Delay

O setup aguarda **3 segundos** antes de ativar a contagem de intersecções:

```cpp
delay(3000);
attachInterrupt(digitalPinToInterrupt(IR_CONTADOR_LINHA), contadorLinhaISR, RISING);
```

Isso permite posicionar o robô na largada sem disparar contagens falsas.

## Estrutura do Repositório

```
robo-agv/
├── robo_agv.ino  # Código principal Arduino
└── README.md     # Este arquivo
```

## Critérios de Avaliação Atendidos

| Critério                              | Pontos | Status |
|---------------------------------------|--------|--------|
| Movimentação pelas linhas             | 1 pt   | ✅     |
| Executar o caminho pré-determinado    | 3 pts  | ✅     |
| Sistema anti-colisão                  | 2 pts  | ✅     |
| Sistema de tomada de decisão          | 2 pts  | ✅     |
| Execução sem interferência humana     | 2 pts  | ✅     |
