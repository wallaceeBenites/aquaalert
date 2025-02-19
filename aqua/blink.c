#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "hardware/clocks.h"
#include "ws2818b.pio.h"


// Definição dos pinos
#define BUZZER_A_PIN 21   // Pino do Buzzer A (GP21)
#define BUZZER_B_PIN 10   // Pino do Buzzer B (GP10)
#define LED_PIN 7  // Pino da Matriz de LED RGB (GP7)
#define LED_COUNT 25
#define BUTTON_ALARM_ON 5  // Botão A (GP5) para ligar alarmes
#define BUTTON_ALARM_OFF 6 // Botão B (GP6) para desligar alarmes

// Codigo referente a matriz de led =================================================================================================================================================

// Definição de pixel GRB
struct pixel_t {
    uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
  };
  typedef struct pixel_t pixel_t;
  typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.
  
  // Declaração do buffer de pixels que formam a matriz.
  npLED_t leds[LED_COUNT];
  
  // Variáveis para uso da máquina PIO.
  PIO np_pio;
  uint sm;
  
  /**
   * Inicializa a máquina PIO para controle da matriz de LEDs.
   */
  void npInit(uint pin) {
  
    // Cria programa PIO.
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
  
    // Toma posse de uma máquina PIO.
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
      np_pio = pio1;
      sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
    }
  
    // Inicia programa na máquina PIO obtida.
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
  
    // Limpa buffer de pixels.
    for (uint i = 0; i < LED_COUNT; ++i) {
      leds[i].R = 0;
      leds[i].G = 0;
      leds[i].B = 0;
    }
  }
  
  /**
   * Atribui uma cor RGB a um LED.
   */
  void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
  }
  
  /**
   * Limpa o buffer de pixels.
   */
  void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i)
      npSetLED(i, 0, 0, 0);
  }
  
  /**
   * Escreve os dados do buffer nos LEDs.
   */
  void npWrite() {
    // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
    for (uint i = 0; i < LED_COUNT; ++i) {
      pio_sm_put_blocking(np_pio, sm, leds[i].G);
      pio_sm_put_blocking(np_pio, sm, leds[i].R);
      pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
  }
  
  int getIndex(int x, int y) {
    // Se a linha for par (0, 2, 4), percorremos da esquerda para a direita.
    // Se a linha for ímpar (1, 3), percorremos da direita para a esquerda.
    if (y % 2 == 0) {
        return 24-(y * 5 + x); // Linha par (esquerda para direita).
    } else {
        return 24-(y * 5 + (4 - x)); // Linha ímpar (direita para esquerda).
    }
  }


// Codigo referente ao alarme =================================================================================================================================================

// Estado do alarme
bool alarme_ativo = false;

// Variáveis para controle do PWM
uint slice_num_a;  // Slice do PWM para o Buzzer A
uint channel_a;    // Canal do PWM para o Buzzer A
uint slice_num_b;  // Slice do PWM para o Buzzer B
uint channel_b;    // Canal do PWM para o Buzzer B

// Função para configurar o PWM nos buzzers
void configurar_pwm() {
    // Configura o pino do Buzzer A como saída PWM
    gpio_set_function(BUZZER_A_PIN, GPIO_FUNC_PWM);
    slice_num_a = pwm_gpio_to_slice_num(BUZZER_A_PIN);
    channel_a = pwm_gpio_to_channel(BUZZER_A_PIN);

    // Configura o pino do Buzzer B como saída PWM
    gpio_set_function(BUZZER_B_PIN, GPIO_FUNC_PWM);
    slice_num_b = pwm_gpio_to_slice_num(BUZZER_B_PIN);
    channel_b = pwm_gpio_to_channel(BUZZER_B_PIN);

    // Configura o PWM para ambos os buzzers
    pwm_set_wrap(slice_num_a, 62500);  // Define a frequência para 2 kHz (125000000 / 62500)
    pwm_set_chan_level(slice_num_a, channel_a, 31250);  // Duty cycle de 50% (31250 / 62500)
    pwm_set_wrap(slice_num_b, 62500);  // Define a frequência para 2 kHz (125000000 / 62500)
    pwm_set_chan_level(slice_num_b, channel_b, 31250);  // Duty cycle de 50% (31250 / 62500)
}

// Função para emitir o padrão de bipes
void emitir_bipes() {

    // Matriz de leds 5x5
    int matriz[5][5][3] = {
        {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
        {{255, 0, 0}, {255, 0, 0}, {255, 255, 255}, {255, 0, 0}, {255, 0, 0}},
        {{255, 0, 0}, {255, 255, 255}, {255, 0, 0}, {255, 255, 255}, {255, 0, 0}},
        {{255, 0, 0}, {255, 0, 0}, {255, 255, 255}, {255, 0, 0}, {255, 0, 0}},
        {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}}
      };
      // Desenhando Sprite contido na matriz.c
    for(int linha = 0; linha < 5; linha++){
      for(int coluna = 0; coluna < 5; coluna++){
        int posicao = getIndex(linha, coluna);
        npSetLED(posicao, matriz[coluna][linha][0], matriz[coluna][linha][1], matriz[coluna][linha][2]);
      }
    }
        
    // Aciona os buzzers e o led
    npWrite();
    pwm_set_enabled(slice_num_a, true);
    pwm_set_enabled(slice_num_b, true);

    // Aguarda por 100ms (duração do beep)
    sleep_ms(500);

    // Desliga os buzzers e o led 
    npClear();
    npWrite();
    pwm_set_enabled(slice_num_a, false);
    pwm_set_enabled(slice_num_b, false);

    // Aguarda 200ms antes do próximo beep
    sleep_ms(200);
}

// Função para ativar o alarme com bipes contínuos
void iniciar_alerta() {
    alarme_ativo = true;
    printf(" Queda de energia detectada!\n");

    // Loop infinito para manter o alarme ativado
    while (alarme_ativo) {

        emitir_bipes();  // Emitir o padrão de bipes e ligar a matriz de led 
        

       

        // Verificar se o botão B foi pressionado para desligar o alarme
        if (gpio_get(BUTTON_ALARM_OFF) == 0) {
            
            parar_alerta();  // Chama a função para desligar o alarme
            break;  // Sai do loop
        }
    }
}

// Função para desativar o alarme
void parar_alerta() {
    // Desabilita o PWM nos buzzers
    pwm_set_enabled(slice_num_a, false);
    pwm_set_enabled(slice_num_b, false);

    // Atualiza o estado do alarme
    alarme_ativo = false;

    // Desativa os Leds 
    npClear();
    npWrite(); 

    printf("✅ Alarmes desativados!\n");
}

int main() {
    stdio_init_all();

    // Inicializa matriz de LEDs NeoPixel.
    npInit(LED_PIN);
    npClear();

    // Aqui, você desenha nos LEDs.

    npWrite(); // Escreve os dados nos LEDs.

    // Inicialização dos pinos
    gpio_init(BUZZER_A_PIN);
    gpio_set_dir(BUZZER_A_PIN, GPIO_OUT);

    gpio_init(BUZZER_B_PIN);
    gpio_set_dir(BUZZER_B_PIN, GPIO_OUT);

    gpio_init(BUTTON_ALARM_ON);
    gpio_set_dir(BUTTON_ALARM_ON, GPIO_IN);
    gpio_pull_up(BUTTON_ALARM_ON);

    gpio_init(BUTTON_ALARM_OFF);
    gpio_set_dir(BUTTON_ALARM_OFF, GPIO_IN);
    gpio_pull_up(BUTTON_ALARM_OFF);

    // Configura o PWM nos buzzers
    configurar_pwm();

    while (1) {
        if (gpio_get(BUTTON_ALARM_ON) == 0 && !alarme_ativo) {  // Se o botão A for pressionado(simula queda de energia) e o alarme estiver desligado 

            iniciar_alerta();
            sleep_ms(300);  // Debounce básico
        }

        sleep_ms(100);
    }
}