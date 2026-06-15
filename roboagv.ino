// Estados
#define SEGUIR 0
#define CURVA_D 1
#define CURVA_E 2
#define PARADO 3
#define PARADO_FIM 4

#define FRENTE 0
#define DIREITA 1
#define ESQUERDA 2
#define FIM 3

// Pinos
#define LED_DEBUG 13
#define IR_CONTADOR_LINHA 3
#define IR_OBSTACULO 2
#define IR_SEGUIDOR_DIREITO 12
#define IR_SEGUIDOR_ESQUERDO 11
#define PWM_MOTOR_DIREITO 5
#define PWM_MOTOR_ESQUERDO 6
#define MOTOR_DIREITO_IN_0 10
#define MOTOR_DIREITO_IN_1 9
#define MOTOR_ESQUERDO_IN_0 8
#define MOTOR_ESQUERDO_IN_1 7

#define V_FRENTE_DIREITO 200
#define V_FRENTE_ESQUERDO 128

#define V_CURVA_D_RAPIDO 170  // roda esquerda na curva direita
#define V_CURVA_D_LENTO 10    // roda direita na curva direita

#define V_CURVA_E_RAPIDO 240  // roda direita na curva esquerda (minimo 200 para girar)
#define V_CURVA_E_LENTO 10    // roda esquerda na curva esquerda

#define T_CURVA_D_MIN 1000  // direita: cego mais longo, sensor detecava linha cedo
#define T_CURVA_E_MIN 1000  // esquerda: cego mais curto, começa a checar antes

#define T_CURVA_D_MAX 7500 // direita: timeout
#define T_CURVA_E_MAX 7500  // esquerda: timeout menor, evita girar de mais

int sequencia[] = {FRENTE, DIREITA, ESQUERDA, FRENTE, FRENTE};
const int totalPassos = 5;

volatile bool obstaculoAtivo = false;

int estado = SEGUIR;
int estadoSalvo = SEGUIR;
int count = 0;

void setup()
{
  Serial.begin(9600);

  pinMode(IR_SEGUIDOR_ESQUERDO, INPUT);
  pinMode(IR_SEGUIDOR_DIREITO, INPUT);
  pinMode(IR_OBSTACULO, INPUT);
  pinMode(IR_CONTADOR_LINHA, INPUT);

  pinMode(PWM_MOTOR_DIREITO, OUTPUT);
  pinMode(PWM_MOTOR_ESQUERDO, OUTPUT);
  pinMode(MOTOR_DIREITO_IN_0, OUTPUT);
  pinMode(MOTOR_DIREITO_IN_1, OUTPUT);
  pinMode(MOTOR_ESQUERDO_IN_0, OUTPUT);
  pinMode(MOTOR_ESQUERDO_IN_1, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(IR_OBSTACULO), obsISR, CHANGE);

  // Aguarda 3s para posicionar o robo antes de comecar a contar interseccoes
  delay(3000);
  attachInterrupt(digitalPinToInterrupt(IR_CONTADOR_LINHA), contadorLinhaISR, RISING);
}

void loop()
{
  if(estado == PARADO_FIM)
  {
    parar();
    return;
  }

  if(obstaculoAtivo)
  {
    if(estado != PARADO)
    {
      estadoSalvo = estado;
      estado = PARADO;
    }
    parar();
    return;
  }

  if(estado == PARADO)
    estado = estadoSalvo;

  switch(estado)
  {
    case SEGUIR: seguir(); break;
    case CURVA_D: curvaDireita(); break;
    case CURVA_E: curvaEsquerda(); break;
    case PARADO: parar(); break;
  }
}

void seguir()
{
  bool L = digitalRead(IR_SEGUIDOR_ESQUERDO);
  bool R = digitalRead(IR_SEGUIDOR_DIREITO);

  if(L == R)
    moverFrente();
  else if(R == HIGH)
    virarDireita();
  else
    virarEsquerda();
}

void curvaDireita()
{
  digitalWrite(MOTOR_DIREITO_IN_0, HIGH); digitalWrite(MOTOR_DIREITO_IN_1, LOW);
  digitalWrite(MOTOR_ESQUERDO_IN_0, HIGH); digitalWrite(MOTOR_ESQUERDO_IN_1, LOW);
  analogWrite(PWM_MOTOR_DIREITO, V_CURVA_D_LENTO);
  analogWrite(PWM_MOTOR_ESQUERDO, V_CURVA_D_RAPIDO);

  delay(T_CURVA_D_MIN);

  unsigned long inicio = millis();
  while(!obstaculoAtivo &&
        digitalRead(IR_SEGUIDOR_ESQUERDO) == LOW &&
        digitalRead(IR_SEGUIDOR_DIREITO) == LOW &&
        millis() - inicio < T_CURVA_D_MAX)
  {
  }

  estado = SEGUIR;
}

void curvaEsquerda()
{
  digitalWrite(MOTOR_DIREITO_IN_0, HIGH); digitalWrite(MOTOR_DIREITO_IN_1, LOW);
  digitalWrite(MOTOR_ESQUERDO_IN_0, HIGH); digitalWrite(MOTOR_ESQUERDO_IN_1, LOW);
  analogWrite(PWM_MOTOR_DIREITO, V_CURVA_E_RAPIDO);
  analogWrite(PWM_MOTOR_ESQUERDO, V_CURVA_E_LENTO);

  delay(T_CURVA_E_MIN);

  unsigned long inicio = millis();
  while(!obstaculoAtivo &&
        digitalRead(IR_SEGUIDOR_ESQUERDO) == LOW &&
        digitalRead(IR_SEGUIDOR_DIREITO) == LOW &&
        millis() - inicio < T_CURVA_E_MAX)
  {
  }

  estado = SEGUIR;
}

void moverFrente()
{
  digitalWrite(MOTOR_DIREITO_IN_0, HIGH); digitalWrite(MOTOR_DIREITO_IN_1, LOW);
  digitalWrite(MOTOR_ESQUERDO_IN_0, HIGH); digitalWrite(MOTOR_ESQUERDO_IN_1, LOW);
  analogWrite(PWM_MOTOR_DIREITO, V_FRENTE_DIREITO);
  analogWrite(PWM_MOTOR_ESQUERDO, V_FRENTE_ESQUERDO);
}

void virarDireita()
{
  // Direita: esquerda rapida, direita parada (abaixo de 200 nao gira)
  digitalWrite(MOTOR_DIREITO_IN_0, HIGH); digitalWrite(MOTOR_DIREITO_IN_1, LOW);
  digitalWrite(MOTOR_ESQUERDO_IN_0, HIGH); digitalWrite(MOTOR_ESQUERDO_IN_1, LOW);
  analogWrite(PWM_MOTOR_DIREITO, 0);
  analogWrite(PWM_MOTOR_ESQUERDO, 210);
}

void virarEsquerda()
{
  // Esquerda: direita rapida (minimo 200), esquerda parada
  digitalWrite(MOTOR_DIREITO_IN_0, HIGH); digitalWrite(MOTOR_DIREITO_IN_1, LOW);
  digitalWrite(MOTOR_ESQUERDO_IN_0, HIGH); digitalWrite(MOTOR_ESQUERDO_IN_1, LOW);
  analogWrite(PWM_MOTOR_DIREITO, 210);
  analogWrite(PWM_MOTOR_ESQUERDO, 0);
}

void parar()
{
  analogWrite(PWM_MOTOR_DIREITO, 0);
  analogWrite(PWM_MOTOR_ESQUERDO, 0);
  digitalWrite(MOTOR_DIREITO_IN_0, LOW); digitalWrite(MOTOR_DIREITO_IN_1, LOW);
  digitalWrite(MOTOR_ESQUERDO_IN_0, LOW); digitalWrite(MOTOR_ESQUERDO_IN_1, LOW);
}

void contadorLinhaISR()
{
  if(estado != SEGUIR) return;

  static unsigned long ultimoCount = 0;
  if(millis() - ultimoCount < 500) return;
  ultimoCount = millis();

  if(count >= totalPassos) return;

  int decisao = sequencia[count];
  count++;

  switch(decisao)
  {
    case DIREITA: estado = CURVA_D; break;
    case ESQUERDA: estado = CURVA_E; break;
    case FIM: estado = PARADO_FIM; break;
  }
}

void obsISR()
{
  obstaculoAtivo = (digitalRead(IR_OBSTACULO) == LOW);
}
