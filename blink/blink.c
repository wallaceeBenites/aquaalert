#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

// Definição dos pinos
#define BUZZER_A_PIN 21   // Pino do Buzzer A (GP21)
#define BUZZER_B_PIN 10   // Pino do Buzzer B (GP10)
#define LED_MATRIX_PIN 7  // Pino da Matriz de LED RGB (GP7)
#define BUTTON_ALARM_ON 5  // Botão A (GP5) para ligar alarmes
#define BUTTON_ALARM_OFF 6 // Botão B (GP6) para desligar alarmes

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
    // Aciona os buzzers
    pwm_set_enabled(slice_num_a, true);
    pwm_set_enabled(slice_num_b, true);
    gpio_put(LED_MATRIX_PIN, 1);  // Liga o LED RGB

    // Aguarda por 100ms (duração do beep)
    sleep_ms(500);

    // Desliga os buzzers
    pwm_set_enabled(slice_num_a, false);
    pwm_set_enabled(slice_num_b, false);
    gpio_put(LED_MATRIX_PIN, 0);  // Desliga o LED RGB

    // Aguarda 200ms antes do próximo beep
    sleep_ms(200);
}

// Função para ativar o alarme com bipes contínuos
void iniciar_alerta() {
    alarme_ativo = true;
    printf("⚠️ Alarmes ativados!\n");

    // Loop infinito para manter o alarme ativado
    while (alarme_ativo) {
        emitir_bipes();  // Emitir o padrão de bipes

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

    // Desliga a matriz de LED RGB
    gpio_put(LED_MATRIX_PIN, 0);

    // Atualiza o estado do alarme
    alarme_ativo = false;
    printf("✅ Alarmes desativados!\n");
}

int main() {
    stdio_init_all();

    // Inicialização dos pinos
    gpio_init(BUZZER_A_PIN);
    gpio_set_dir(BUZZER_A_PIN, GPIO_OUT);

    gpio_init(BUZZER_B_PIN);
    gpio_set_dir(BUZZER_B_PIN, GPIO_OUT);

    gpio_init(LED_MATRIX_PIN);
    gpio_set_dir(LED_MATRIX_PIN, GPIO_OUT);

    gpio_init(BUTTON_ALARM_ON);
    gpio_set_dir(BUTTON_ALARM_ON, GPIO_IN);
    gpio_pull_up(BUTTON_ALARM_ON);

    gpio_init(BUTTON_ALARM_OFF);
    gpio_set_dir(BUTTON_ALARM_OFF, GPIO_IN);
    gpio_pull_up(BUTTON_ALARM_OFF);

    // Configura o PWM nos buzzers
    configurar_pwm();

    while (1) {
        if (gpio_get(BUTTON_ALARM_ON) == 0 && !alarme_ativo) {  // Se o botão A for pressionado e o alarme estiver desligado
            iniciar_alerta();
            sleep_ms(300);  // Debounce básico
        }

        if (gpio_get(BUTTON_ALARM_OFF) == 0 && alarme_ativo) {  // Se o botão B for pressionado e o alarme estiver ligado
            parar_alerta();
            sleep_ms(300);  // Debounce básico
        }

        sleep_ms(100);
    }
}