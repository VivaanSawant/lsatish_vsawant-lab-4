/**
 * @file servok.c
 *
 * @brief      Software PWM for two servo channels, driven by one TIM2
 *             interrupt every 10 us.
 *
 *             Each 20 ms frame is 2000 ticks. At tick 0 every enabled
 *             channel's pin goes high, and it goes low once the tick count
 *             reaches that channel's pulse width. Pulse widths are computed
 *             in sys_servo_set so the interrupt handler only counts and
 *             compares.
 *
 *             Channels are numbered 1 and 2 to match the handout's console:
 *               channel 1 -> PA8 (Arduino D7)
 *               channel 2 -> PB10 (Arduino D6)
 *
 * @date       2026-10-26
 *
 * @author     Vivaan Sawant (vsawant@andrew.cmu.edu)
 */

#include <unistd.h>
#include <stdint.h>
#include <gpio.h>
#include <timer.h>
#include <syscall.h>
#include <servok.h>

/** @brief Timer used for the PWM tick */
#define SERVO_TIMER 2

/** @brief 16 MHz / 16 = 1 MHz counter clock; 10 counts = one 10 us tick */
#define SERVO_TIMER_PRESCALER 16
#define SERVO_TIMER_PERIOD    10

/** @brief 20 ms frame / 10 us tick */
#define TICKS_PER_FRAME 2000

/** @brief Pulse width endpoints in ticks: 0.6 ms at 0 deg, 2.4 ms at 180 deg */
#define PULSE_MIN_TICKS 60
#define PULSE_MAX_TICKS 240
#define ANGLE_MAX       180

/** @brief User-facing channel numbers */
#define SERVO_CH1 1
#define SERVO_CH2 2

/** @brief Pins wired to each servo's orange signal wire */
#define SERVO_CH1_PORT GPIO_A
#define SERVO_CH1_PIN  8    // D7
#define SERVO_CH2_PORT GPIO_B
#define SERVO_CH2_PIN  10   // D6

/** @brief Pulse width in ticks for channel 1 / channel 2 */
int pwm_channel0 = PULSE_MIN_TICKS;
int pwm_channel1 = PULSE_MIN_TICKS;

/** @brief Nonzero while channel 1 / channel 2 is outputting pulses */
int ch0_en = 0;
int ch1_en = 0;

/** @brief Position within the current 20 ms frame, in ticks */
static uint32_t frame_tick = 0;

/**
 * @brief Converts an angle to a pulse width in ticks.
 *
 * @param angle 0 to 180 degrees
 *
 * @return pulse width in ticks
 */
static int angle_to_ticks(uint8_t angle) {
  return PULSE_MIN_TICKS +
         (angle * (PULSE_MAX_TICKS - PULSE_MIN_TICKS)) / ANGLE_MAX;
}

/**
 * @brief Configures both PWM pins as outputs held low and starts the
 *        10 us PWM timer. Called on the first sys_servo_enable.
 */
void servo_init(void) {
  gpio_init(SERVO_CH1_PORT, SERVO_CH1_PIN, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL,
            OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
  gpio_init(SERVO_CH2_PORT, SERVO_CH2_PIN, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL,
            OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
  gpio_clr(SERVO_CH1_PORT, SERVO_CH1_PIN);
  gpio_clr(SERVO_CH2_PORT, SERVO_CH2_PIN);

  timer_init(SERVO_TIMER, SERVO_TIMER_PRESCALER, SERVO_TIMER_PERIOD);
}

/**
 * @brief TIM2 interrupt: advances the frame and toggles pins only on edges.
 *
 * Rising edge for every enabled channel at tick 0, falling edge when the
 * tick count reaches that channel's pulse width. Every other tick just
 * counts, which keeps the 100 kHz handler cheap.
 */
void servo_timer_handler(void) {
  timer_clear_interrupt_bit(SERVO_TIMER);

  if (frame_tick == 0) {
    if (ch0_en) gpio_set(SERVO_CH1_PORT, SERVO_CH1_PIN);
    if (ch1_en) gpio_set(SERVO_CH2_PORT, SERVO_CH2_PIN);
  }
  if (frame_tick == (uint32_t)pwm_channel0) gpio_clr(SERVO_CH1_PORT, SERVO_CH1_PIN);
  if (frame_tick == (uint32_t)pwm_channel1) gpio_clr(SERVO_CH2_PORT, SERVO_CH2_PIN);

  if (++frame_tick == TICKS_PER_FRAME) frame_tick = 0;
}

/**
 * @brief Enables or disables pulses on a channel. A disabled channel's pin
 *        is held low so the servo can be turned by hand.
 *
 * @param channel 1 or 2
 * @param enabled nonzero to enable, 0 to disable
 *
 * @return 0 on success, -1 for an invalid channel
 */
int sys_servo_enable(uint8_t channel, uint8_t enabled) {
  /* started lazily: a 100 kHz interrupt from boot would skew thread timing */
  static int servo_started = 0;
  if (!servo_started) {
    servo_init();
    servo_started = 1;
  }

  if (channel == SERVO_CH1) {
    ch0_en = enabled;
    if (!enabled) gpio_clr(SERVO_CH1_PORT, SERVO_CH1_PIN);
  } else if (channel == SERVO_CH2) {
    ch1_en = enabled;
    if (!enabled) gpio_clr(SERVO_CH2_PORT, SERVO_CH2_PIN);
  } else {
    return -1;
  }
  return 0;
}

/**
 * @brief Stores the pulse width for an angle. Does not enable the channel;
 *        a disabled channel picks up the new angle once it is enabled.
 *
 * @param channel 1 or 2
 * @param angle   0 to 180 degrees
 *
 * @return 0 on success, -1 for an invalid channel or angle
 */
int sys_servo_set(uint8_t channel, uint8_t angle) {
  if (angle > ANGLE_MAX) return -1;

  if (channel == SERVO_CH1) {
    pwm_channel0 = angle_to_ticks(angle);
  } else if (channel == SERVO_CH2) {
    pwm_channel1 = angle_to_ticks(angle);
  } else {
    return -1;
  }
  return 0;
}